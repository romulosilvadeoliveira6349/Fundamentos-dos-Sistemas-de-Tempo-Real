#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>


#define FALHA 1

#define	TAM_MEU_BUFFER	1000




int cria_socket_local(void)
{
	int socket_local;		// Socket usado na comunicacao

	socket_local = socket( PF_INET, SOCK_DGRAM, 0);
	if (socket_local < 0) {
		perror("socket");
		return -1;
	}
	return socket_local;
}





struct sockaddr_in cria_endereco_destino(char *destino, int porta_destino)
{
	struct sockaddr_in servidor; 	// Endereco do servidor incluindo ip e porta
	struct hostent *dest_internet;	// Endereco destino em formato proprio
	struct in_addr dest_ip;			// Endereco destino em formato ip numerico

	if (inet_aton ( destino, &dest_ip ))
		dest_internet = gethostbyaddr((char *)&dest_ip, sizeof(dest_ip), AF_INET);
	else
		dest_internet = gethostbyname(destino);

	if (dest_internet == NULL) {
		fprintf(stderr,"Endereço de rede inválido\n");
		exit(FALHA);
	}

	memset((char *) &servidor, 0, sizeof(servidor));
	memcpy(&servidor.sin_addr, dest_internet->h_addr_list[0], sizeof(servidor.sin_addr));
	servidor.sin_family = AF_INET;
	servidor.sin_port = htons(porta_destino);

	return servidor;
}




void envia_mensagem(int socket_local, struct sockaddr_in endereco_destino, char *mensagem)
{
	/* Envia msg ao servidor */

	if (sendto(socket_local, mensagem, strlen(mensagem)+1, 0, (struct sockaddr *) &endereco_destino, sizeof(endereco_destino)) < 0 )
	{
		perror("sendto");
		return;
	}
}


int recebe_mensagem(int socket_local, char *buffer, int TAM_BUFFER)
{
	int bytes_recebidos;		/* Número de bytes recebidos */

	/* Espera pela msg de resposta do servidor */
	bytes_recebidos = recvfrom(socket_local, buffer, TAM_BUFFER, 0, NULL, 0);
	if (bytes_recebidos < 0)
	{
		perror("recvfrom");
	}

	return bytes_recebidos;
}





int main(int argc, char *argv[])
{
	if (argc < 3) {
		fprintf(stderr,"Uso: cliente <endereco> <porta> \n");
		fprintf(stderr,"<endereco> eh o DNS ou IP do servidor \n");
		fprintf(stderr,"<porta> eh o numero da porta do servidor \n");
		exit(FALHA);
	}

	int porta_destino = atoi( argv[2]);

	int socket_local = cria_socket_local();

	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 100000;	// 100 ms
	if( setsockopt(socket_local, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout) ) < 0 ) {
	    perror("Socket local");
		exit(FALHA);
	}

	struct sockaddr_in endereco_destino1 = cria_endereco_destino(argv[1], porta_destino);
	struct sockaddr_in endereco_destino2 = cria_endereco_destino(argv[1], porta_destino+1);


	int i = 0;
	char msg_enviada[1000];
	char msg_recebida[1000];
	int nrec1, nrec2;

	while(1) {
		double lido1 = -1;
		double lido2 = -1;
		printf("\n\nTentativa %d\n", i);

		envia_mensagem(socket_local, endereco_destino1, "st-0");
		nrec1 = recebe_mensagem(socket_local, msg_recebida, 1000);
		if( nrec1 > 0 ) {
			msg_recebida[nrec1] = '\0';
			printf("Mensagem de resposta [1] com %d bytes >>>%s<<<\n", nrec1, msg_recebida);
			lido1 = atof( &msg_recebida[3] );
		}

		envia_mensagem(socket_local, endereco_destino2, "st-0");
		nrec2 = recebe_mensagem(socket_local, msg_recebida, 1000);
		if( nrec2 > 0 ) {
			msg_recebida[nrec2] = '\0';
			printf("Mensagem de resposta [2] com %d bytes >>>%s<<<\n", nrec2, msg_recebida);
			lido2 = atof( &msg_recebida[3] );
		}

		if( nrec1 > 0  &&  nrec2 > 0 )
			printf("Valor %lf aceito\n", (lido1+lido2)/2);
		else if( nrec1 > 0  &&  nrec2 <= 0 )
			printf("Valor %lf aceito\n", lido1);
		else if( nrec1 <= 0  &&  nrec2 > 0 )
			printf("Valor %lf aceito\n", lido2);
		else
			printf("Valor nao aceito\n");

		sleep(3);
		++i;
	}

}




