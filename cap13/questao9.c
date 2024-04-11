#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "q9sensores.h"

pthread_t tEscTipoA, tEscTipoB, tLeTipoA, tLeTipoB, tEspera;

int continuar = 1;

void escTipoA(void) {
	struct dados_medidos_t x;
	while(continuar) {
		sleep(2);
		printf("Thread escTipoA insere medicao\n");
		insere_medicao( TIPO_A,x);
	}
}


void escTipoB(void) {
	struct dados_medidos_t x;
	while(continuar) {
		sleep(5);
		printf("Thread escTipoB insere medicao\n");
		insere_medicao( TIPO_B,x);
	}
}



double leTipoA( void) {
	struct dados_medidos_t x;
	printf("Thread leTipoA tenta ler\n");
	x = consulta_medicao( TIPO_A);
	printf("Thread leTipoA consegue ler\n");
}


double leTipoB( void) {
	struct dados_medidos_t x;
	printf("Thread leTipoB tenta ler\n");
	x = consulta_medicao( TIPO_B);
	printf("Thread leTipoB consegue ler\n");
}


void espera(void) {
	printf("Thread espera vai esperar pelo alarme\n");
	espera_alarme();
	printf("Thread espera sai da espera do alarme\n");
	sleep(1);
	continuar = 0;
}



void main(void) {

	pthread_create( &tEscTipoA, NULL, (void *)escTipoA, NULL);
	pthread_create( &tEscTipoB, NULL, (void *)escTipoB, NULL);
	pthread_create( &tLeTipoA, NULL, (void *)leTipoA, NULL);
	pthread_create( &tLeTipoB, NULL, (void *)leTipoB, NULL);
	pthread_create( &tEspera, NULL, (void *)espera, NULL);

	pthread_join( tEscTipoA, NULL );
	pthread_join( tEscTipoB, NULL);
	pthread_join( tLeTipoA, NULL);
	pthread_join( tLeTipoB, NULL);
	pthread_join( tEspera, NULL);
}



