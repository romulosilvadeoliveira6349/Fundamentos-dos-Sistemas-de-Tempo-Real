#include	<sys/types.h>
#include	<sys/socket.h>
#include	<string.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<stdio.h>
#include	<errno.h>
#include	<stdlib.h>
#include	<netdb.h>
#include	<time.h>
#include	<assert.h>
#include	<pthread.h>

#include	"instrumentacao.h"

#include	"solar2022.h"
#include	"planta.h"


#define		TAM_BUFFER		10000


// Portas UDP/IP
static int portaUDPpInstrumentacao;		// Porta UDP para acessar a instrumentação
static int portaUDPpNuvem;				// Porta UDP usada para enviar valor de referencia de temperatura


// Sockets UDP/IP
static int socketInstrumentacao;		// Socket para a instrumentação
static int socketNuvem;					// Socket para enviar valor de referencia de temperatura


// Endereço do controlador para mandar comandos como se fosse outro computador
static struct sockaddr_in enderecoControlador;		// Computador e porta



/********************************************************
*														*
*	FUNÇÕES PARA ACESSO A INTERNET						*
*														*
********************************************************/



/**	Cria socket local para UDP/IP
*	Retorna -1 em caso de falha
*/
static int criaSocketLocal(void)
{
	int socket_local;
	socket_local = socket( AF_INET, SOCK_DGRAM, 0);
	if (socket_local < 0) {
		perror("socket");
		exit(-1);
	}
	return socket_local;
}

/**	Define porta escutada por um socket (lado servidor)
*/
static void define_porta_escutada(int socket_local, int porta_escutada)
{
	struct sockaddr_in servidor;	// Endereço completo do servidor e do cliente
	int tam_s;						// Tamanho da estrutura servidor
	
	memset((char *) &servidor, 0, sizeof(servidor));
	servidor.sin_family = AF_INET;
	servidor.sin_addr.s_addr = htonl(INADDR_ANY);
	servidor.sin_port = htons(porta_escutada);

	tam_s = sizeof(servidor);
	if (bind(socket_local, (struct sockaddr *) &servidor, tam_s) < 0) {
		perror("bind");
		exit(-1);
	}
}


/**	Recebe mensagem de um cliente (lado servidor)
*	Retorna número de bytes recebidos
*/
static int recebe_mensagem( char *buffer, int tam_buffer, int socket_local, struct sockaddr_in *endereco_cliente, unsigned *tam_c)
{
	int bytes_recebidos;			// Número de bytes recebidos

	memset( (char *)endereco_cliente, 0, sizeof(struct sockaddr_in));
	endereco_cliente->sin_family = AF_INET;

	*tam_c = sizeof(struct sockaddr_in);
	bytes_recebidos = recvfrom(socket_local, buffer, tam_buffer, 0, (struct sockaddr *) endereco_cliente, tam_c);
	if (bytes_recebidos < 0) {
		perror("recvfrom");
		exit(-1);
	}
	return bytes_recebidos;
}



/**	Envia mensagem de resposta para o cliente (lado servidor)
*/
static void responde_mensagem(char *mensagem, int socket_local, struct sockaddr_in *endereco_cliente, unsigned tam_c )
{
	if (sendto(socket_local, mensagem, strlen(mensagem)+1, 0, (struct sockaddr *) endereco_cliente, tam_c) < 0) {
		perror("sendto");
		exit(-1);
	}
}



/** Monta o endereço do servidor para mandar uma mensagem (lado cliente)
*/
static struct sockaddr_in criaEnderecoDestino(char *computadorDestino, int portaDestino)
{
	struct sockaddr_in servidor;		// Endereço do servidor incluindo ip e porta
	struct hostent *dest_internet;		// Endereço destino em formato próprio
	struct in_addr dest_ip;				// Endereço destino em formato ip numérico

	if (inet_aton ( computadorDestino, &dest_ip ))
		dest_internet = gethostbyaddr((char *)&dest_ip, sizeof(dest_ip), AF_INET);
	else
		dest_internet = gethostbyname(computadorDestino);

	if (dest_internet == NULL) {
		fprintf(stderr,"Endereço de rede inválido\n");
		exit(-1);
	}

	memset((char *) &servidor, 0, sizeof(servidor));
	memcpy(&servidor.sin_addr, dest_internet->h_addr_list[0], sizeof(servidor.sin_addr));
	servidor.sin_family = AF_INET;
	servidor.sin_port = htons(portaDestino);

	return servidor;
}



/** Envia uma mensagem (lado cliente)
*/
static void enviaMensagem(int socket_local, struct sockaddr_in endereco_destino, char *mensagem)
{
	if (sendto(socket_local, mensagem, strlen(mensagem)+1, 0, (struct sockaddr *) &endereco_destino, sizeof(endereco_destino)) < 0 ) { 
		perror("sendto");
		return;
	}
}



/** Recebe uma mensagem (lado cliente)
*
static int recebeMensagem(int socket_local, char *buffer, int tamBuffer)
{
	int nBytes;		// Numero de bytes recebidos

	nBytes = recvfrom(socket_local, buffer, tamBuffer, 0, NULL, NULL);
	return nBytes;
}
*/


	/********************************************
	*											*
	*	THREAD SERVIDORA DA INSTRUMENTAÇÃO		*
	*											*
	********************************************/



/**	Separa uma mensagem em campos
*	Retorna número de campos encontrados
*/
static int separaCampos( char *buffer, char *campos[], int maxCampos)
{
	char *token;
	int i = 0;
	int j;

	// Pega o primeiro token
	token = strtok(buffer," ");
	campos[i++] = token;

	while( token != NULL  &&  i<maxCampos ) {
		token = strtok(NULL, " ");		// Pega o próximo token
		campos[i++] = token;
	}
	j = i-1;

	while( i < maxCampos )
		campos[i++] = NULL;			// Completa com NULL

	return j;
}



/**	Thread servidora que recebe comandos para a instrumentacao
*	Formato do protocolo é string com campos separados por espaços
*	Responde ? quando não entendeu o argumento, exemplo "bombacoletor ?"
*
*	Pedido para ligar/desligar a bomba do coletor
*		"bombacoletor"
*		<novo estado: 1 ou 0>
*	Resposta 
*		"zbombacoletor"
*		<novo estado: 1 ou 0>
*
*
*	Pedido para ligar/desligar a bomba de circulacao
*		"bombacirculacao"
*		<novo estado: 1 ou 0>
*	Resposta 
*		"zbombacirculacao"
*		<novo estado: 1 ou 0>
*
*
*	Pedido para ligar/desligar o aquecedor eletrico
*		"aquecedor"
*		<novo estado: 1 ou 0>
*	Resposta 
*		"zaquecedor"
*		<novo estado: 1 ou 0>
*
*
*	Pedido para ligar/desligar a válvula de entrada
*		"valvulaentrada"
*		<novo estado: 1 ou 0>
*	Resposta 
*		"zvalvulaentrada"
*		<novo estado: 1 ou 0>
*
*
*	Pedido para ligar/desligar a válvula de esgoto
*		"valvulaesgoto"
*		<novo estado: 1 ou 0>
*	Resposta 
*		"zvalvulaesgoto"
*		<novo estado: 1 ou 0>
*
*
*	Pedido para ler o nível do boiler
*		"nivelboiler"
*	Resposta 
*		"znivelboiler"
*		<nivel:float>			nível em metros
*
*
*	Pedido para ler a temperatura do boiler
*		"tempboiler"
*	Resposta
*		"ztempboiler"
*		<temperatura:float>		temperatura em graus celsius
*
*
*	Pedido para ler a temperatura do coletor
*		"tempcoletor"
*	Resposta
*		"ztempcoletor"
*		<temperatura:float>		temperatura em graus celsius
*
*
*	Pedido para ler a temperatura dos canos
*		"tempcanos"
*	Resposta
*		"ztempcanos"
*		<temperatura:float>		temperatura em graus celsius
*
*/
void instrumentacao_threadServidora(void)
{
	int nBytes;
	char bufferIn[TAM_BUFFER];
	char bufferOut[TAM_BUFFER];
	struct sockaddr_in clientAddress;
	unsigned tam_c;
	char *campos[10];

	printf("instrumentacao_threadServidora criada ...\n");

	while( !solar2022_termina  ) {
		nBytes = recebe_mensagem( bufferIn, TAM_BUFFER, socketInstrumentacao, &clientAddress, &tam_c);
		bufferIn[nBytes] = '\0';
		
		int nCampos = separaCampos( bufferIn, campos, 10);
		
		//printf("###### servidora nCampos %d %s %s\n", nCampos, campos[0], campos[1]);

		if( strcmp( campos[0], "bombacoletor") == 0 ) {
			if( nCampos != 2 ) {
				responde_mensagem( "zbombacoletor ?", socketInstrumentacao, &clientAddress, tam_c);
				continue;
			}
			int comando = atoi( campos[1] );
			if( comando < 0  ||  comando > 1 ) {
				responde_mensagem( "zbombacoletor ?", socketInstrumentacao, &clientAddress, tam_c);
				continue;
			}
			planta_acionaBombaColetor(comando);
			sprintf( bufferOut, "zbombacoletor %d", comando);
			responde_mensagem( bufferOut, socketInstrumentacao, &clientAddress, tam_c);
		}

		else if( strcmp( campos[0], "bombacirculacao") == 0 ) {
			if( nCampos != 2 ) {
				responde_mensagem( "zbombacirculacao ?", socketInstrumentacao, &clientAddress, tam_c);
				continue;
			}
			int comando = atoi( campos[1] );
			if( comando < 0  ||  comando > 1 ) {
				responde_mensagem( "zbombacirculacao ?", socketInstrumentacao, &clientAddress, tam_c);
				continue;
			}
			planta_acionaBombaCirculacao(comando);
			sprintf( bufferOut, "zbombacirculacao %d", comando);
			responde_mensagem( bufferOut, socketInstrumentacao, &clientAddress, tam_c);
		}

		else if( strcmp( campos[0], "aquecedor") == 0 ) {
			if( nCampos != 2 ) {
				responde_mensagem( "zaquecedor ?", socketInstrumentacao, &clientAddress, tam_c);
				continue;
			}
			int comando = atoi( campos[1] );
			if( comando < 0  ||  comando > 1 ) {
				responde_mensagem( "zaquecedor ?", socketInstrumentacao, &clientAddress, tam_c);
				continue;
			}
			planta_acionaAquecedor(comando);
			sprintf( bufferOut, "zaquecedor %d", comando);
			responde_mensagem( bufferOut, socketInstrumentacao, &clientAddress, tam_c);
		}

		else if( strcmp( campos[0], "valvulaentrada") == 0 ) {
			if( nCampos != 2 ) {
				responde_mensagem( "zvalvulaentrada ?", socketInstrumentacao, &clientAddress, tam_c);
				continue;
			}
			int comando = atoi( campos[1] );
			if( comando < 0  ||  comando > 1 ) {
				responde_mensagem( "zvalvulaentrada ?", socketInstrumentacao, &clientAddress, tam_c);
				continue;
			}
			planta_acionaValvulaEntrada(comando);
			sprintf( bufferOut, "zvalvulaentrada %d", comando);
			responde_mensagem( bufferOut, socketInstrumentacao, &clientAddress, tam_c);
		}

		else if( strcmp( campos[0], "valvulaesgoto") == 0 ) {
			if( nCampos != 2 ) {
				responde_mensagem( "zvalvulaesgoto ?", socketInstrumentacao, &clientAddress, tam_c);
				continue;
			}
			int comando = atoi( campos[1] );
			if( comando < 0  ||  comando > 1 ) {
				responde_mensagem( "zvalvulaesgoto ?", socketInstrumentacao, &clientAddress, tam_c);
				continue;
			}
			planta_acionaValvulaEsgoto(comando);
			sprintf( bufferOut, "zvalvulaesgoto %d", comando);
			responde_mensagem( bufferOut, socketInstrumentacao, &clientAddress, tam_c);
		}

		else if( strcmp( campos[0], "nivelboiler") == 0 ) {
			if( nCampos != 1 ) {
				responde_mensagem( "znivelboiler ?", socketInstrumentacao, &clientAddress, tam_c);
				continue;
			}
			double nivelboiler = planta_leNivelBoiler();
			sprintf( bufferOut, "znivelboiler %0.3lf", nivelboiler);
			responde_mensagem( bufferOut, socketInstrumentacao, &clientAddress, tam_c);
		}


		else if( strcmp( campos[0], "tempboiler") == 0 ) {
			if( nCampos != 1 ) {
				responde_mensagem( "ztempboiler ?", socketInstrumentacao, &clientAddress, tam_c);
				continue;
			}
			double tempBoiler = planta_leTempBoiler();
			sprintf( bufferOut, "ztempboiler %0.3lf", tempBoiler);
			responde_mensagem( bufferOut, socketInstrumentacao, &clientAddress, tam_c);
		}


		else if( strcmp( campos[0], "tempcoletor") == 0 ) {
			if( nCampos != 1 ) {
				responde_mensagem( "ztempcoletor ?", socketInstrumentacao, &clientAddress, tam_c);
				continue;
			}
			double tempColetor = planta_leTempColetor();
			sprintf( bufferOut, "ztempcoletor %0.3lf", tempColetor);
			responde_mensagem( bufferOut, socketInstrumentacao, &clientAddress, tam_c);
		}

		else if( strcmp( campos[0], "tempcanos") == 0 ) {
			if( nCampos != 1 ) {
				responde_mensagem( "ztempcanos ?", socketInstrumentacao, &clientAddress, tam_c);
				continue;
			}
			double tempCanos = planta_leTempCanos();
			sprintf( bufferOut, "ztempcanos %0.3lf", tempCanos);
			responde_mensagem( bufferOut, socketInstrumentacao, &clientAddress, tam_c);
		}

		else {
			responde_mensagem( "?????", socketInstrumentacao, &clientAddress, tam_c);	
		}
	}	
}



	/************************************************************************
	*																		*
	*	ENVIA MENSAGEM PARA O CONTROLADOR COMO SE FOSSE OUTRO COMPUTADOR	*
	*																		*
	************************************************************************/


/** Envia string para o controlador sem esperar resposta
*/
void instrumentacao_enviaControlador(char *msg)
{
	enviaMensagem( socketNuvem, enderecoControlador, msg);
}



	/************************
	*						*
	*	INICIALIZACAO		*
	*						*
	************************/



/**	Inicializa o módulo de instrumentação
*	porta - número da porta UDP para escutar como servidor
*			porta+1 é escutada pelo controlador para receber comandos da nuvem
*/
void instrumentacao_inicializa( int porta)
{
	// Portas usadas
	portaUDPpInstrumentacao = porta;
	portaUDPpNuvem = porta+1;

	// Sockets
	socketInstrumentacao = criaSocketLocal();
	socketNuvem = criaSocketLocal();

	// Precisa dos sockets para funcionar
	if( socketInstrumentacao == -1  ||  socketNuvem == -1 )
		panico("Falha criação de socket","\n");

	// Como servidor escuta a porta da instrumentação
	define_porta_escutada(socketInstrumentacao, portaUDPpInstrumentacao);
	
	// Como cliente manda comando para a porta do controlador
	enderecoControlador = criaEnderecoDestino( "localhost", portaUDPpNuvem);

}







