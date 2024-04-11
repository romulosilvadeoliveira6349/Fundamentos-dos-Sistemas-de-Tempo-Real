#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<pthread.h>


#include "console1.h"


static pthread_t th_contador, th_teclado, th_status;		// Programa usa três threads

static int terminar = 0;					// 1 = deve terminar o programa

static int valorLido = 0;					// Variável da aplicação

static pthread_mutex_t emValorLido = PTHREAD_MUTEX_INITIALIZER;




/** Código da thread que conta
*/
static void incrementaContagem(void)
{
	while( terminar == 0 ) {
			sleep(1);
			pthread_mutex_lock( &emValorLido);
			++valorLido;
			pthread_mutex_unlock( &emValorLido);
	}
}



/** Código da thread que mostra a situacao na tela
*/
static void showStatus(void)
{
	while( terminar == 0 ) {
			sleep(1);
			console_pegaTela();
			pthread_mutex_lock( &emValorLido);
			printf("Valor lido %d, digite <enter> para comandos\n", valorLido);
			pthread_mutex_unlock( &emValorLido);
			printf("\n");
			console_largaTela();
	}
}




/** Código da thread para interface via teclado
*/
static void leTeclado(void)
{
	char buffer[2000];

	while( terminar == 0 ) {
		int modoEntrada;
		int lido;
		
		fgets(buffer, 2000 , stdin);	// Espera um <enter>
		console_pegaTela();

		modoEntrada = 1;

		while( modoEntrada ) {
			printf("Digite novo valor  ou  'c' p/cancelar  ou  'x' p/terminar:\n");
			fgets(buffer, 2000 , stdin);

			if( buffer[0] == 'c' )
				break;

			if( buffer[0] == 'x' ) {
				terminar = 1;
				break;
			}

			lido = atoi(buffer);

			if( lido < 0  ||  lido > 100 )
				printf("Valor %d invalido, deve estar entre 0 e 100\n", lido);
			else {
				pthread_mutex_lock( &emValorLido);
				valorLido = lido;
				pthread_mutex_unlock( &emValorLido);
				modoEntrada = 0;
			}
		}

		console_largaTela();
	}
}	



/** Main, faz a criação das threads
*/
void main(void) 
{
	printf("Inicio do programa\n");
	
	// Cria threads para interface via console
	printf("Criando threads para interface via console ...\n");
	

	pthread_create( &th_contador, NULL, (void *) incrementaContagem, NULL);
	pthread_create( &th_status, NULL, (void *) showStatus, NULL);
	pthread_create( &th_teclado, NULL, (void *) leTeclado, NULL);


	// Espera thread teclado terminar
	pthread_join( th_teclado, NULL);

	printf("Termina o programa\n");

}



	

