#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "q10barreira.h"

pthread_t thTrabalhadores[N_TRABALHADORES];

pthread_t thFinalizador;



int continuar = 1;

void trabalhador(int *id) {
	int meuId = *id;
	printf("Trabalhador com id %d foi ativado.\n", meuId);

	sleep( meuId );		// Trabalho
	acabei( meuId );

	printf("Trabalhador com id %d terminou.\n", meuId);
}


void finalizador(void) {
	printf("Finalizador foi ativado.\n");
	sleep( 5 );
	espera_todos();
}



void main(void) {
	int ids[N_TRABALHADORES];

	for( int i=0; i<N_TRABALHADORES; ++i)
		ids[i] = i;

	for( int i=0; i<N_TRABALHADORES; ++i)
		pthread_create( &thTrabalhadores[i], NULL, (void *)trabalhador, &ids[i]);

	pthread_create( &thFinalizador, NULL, (void *)finalizador, NULL);


	for( int i=0; i<N_TRABALHADORES; ++i)
		pthread_join( thTrabalhadores[i], NULL);
	pthread_join( thFinalizador, NULL);

}



