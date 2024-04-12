#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#define FALHA 1

#define	TAM_MEU_BUFFER	1000



int cria_socket_local(void)
{
	int socket_local;			/* Socket usado nesta máquina */

	socket_local = socket( AF_INET, SOCK_DGRAM, 0);
	if (socket_local < 0) {
		perror("socket");
		exit(FALHA);
	}

	return socket_local;
}



void define_porta_escutada(int socket_local, int porta_escutada)
{
	struct sockaddr_in servidor; 	/* Endereço completo do servidor e do cliente */
	int tam_s;						/* Tamanho da estrutura servidor */

	memset((char *) &servidor, 0, sizeof(servidor));
	servidor.sin_family = AF_INET;
	servidor.sin_addr.s_addr = htonl(INADDR_ANY);
	servidor.sin_port = htons(porta_escutada);

	tam_s = sizeof(servidor);
	if (bind(socket_local, (struct sockaddr *) &servidor, tam_s) < 0) {
		perror("bind");
		exit(FALHA);
	}
}



int recebe_mensagem( char *buffer, int tam_buffer, int socket_local, struct sockaddr_in *endereco_cliente, int *tam_c)
{
	int bytes_recebidos;			/* Número de bytes recebidos */

	memset( (char *)endereco_cliente, 0, sizeof(struct sockaddr_in));
	endereco_cliente->sin_family = AF_INET;

	*tam_c = sizeof(struct sockaddr_in);
	bytes_recebidos = recvfrom(socket_local, buffer, tam_buffer, 0, (struct sockaddr *) endereco_cliente, tam_c);
	if (bytes_recebidos < 0) {
		perror("recvfrom");
		exit(FALHA);
	}
	return bytes_recebidos;
}




void envia_mensagem(char *mensagem, int socket_local, struct sockaddr_in *endereco_cliente, int tam_c )
{
	if (sendto(socket_local, mensagem, strlen(mensagem)+1, 0, (struct sockaddr *) endereco_cliente, tam_c) < 0) {
		perror("sendto");
		exit(FALHA);
	}
}





int main(int argc, char *argv[])
{
	int porta_escutada;

	if (argc != 2 ) {
		fprintf(stderr,"Uso: teste-nuvem <porta_escutada>\n");
		fprintf(stderr,"<porta_escutada> eh o valor numerico da porta IP escutada\n");
		exit(FALHA);
	}

	porta_escutada = atoi(argv[1]);

	int socket_local = cria_socket_local();
	define_porta_escutada(socket_local, porta_escutada);

	int bytes_recebidos;
	char buffer[TAM_MEU_BUFFER];

	struct sockaddr_in endereco_cliente;
	int tam_c;

	do{
		printf("Esperando mensagem com valor de referencia...\n");
		bytes_recebidos = recebe_mensagem( buffer, TAM_MEU_BUFFER, socket_local, &endereco_cliente, &tam_c);
		buffer[bytes_recebidos] = '\0';
		printf("Recebido: '%s' \n", buffer);
	}while( 1 );

}



