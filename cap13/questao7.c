#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "q7balcao.h"

#define N_THREADS 5
pthread_t compradores[N_THREADS];
pthread_t vendedores[N_THREADS];

void comprador(void)
{
	for( int i=0; i<10; ++i) {
		double x = 1 + rand() % 1000;
		double y = ofertaCompra( x );
		printf("Oferta compra %lf resultou em %lf\n", x, y);
		//sleep(1);
	}	
}


void vendedor(void)
{
	for( int i=0; i<10; ++i) {
		double x = 1 + rand() % 1000;
		double y = ofertaVenda( x );
		printf("Oferta venda %lf resultou em %lf\n", x, y);
		//sleep(1);
	}	
}


void main(void) {

	for( int i = 0; i < N_THREADS; ++i )
		pthread_create( &compradores[i], NULL, (void *)comprador, NULL);
		
	for( int i = 0; i < N_THREADS; ++i )
		pthread_create( &vendedores[i], NULL, (void *)vendedor, NULL);
		
	for( int i = 0; i < N_THREADS; ++i )
		pthread_join( compradores[i], NULL);
		
	for( int i = 0; i < N_THREADS; ++i )
		pthread_join( vendedores[i], NULL);

}



