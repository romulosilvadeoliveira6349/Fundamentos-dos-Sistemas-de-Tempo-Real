#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "q2sensores.h"

pthread_t tEscTensao, tEscCorrente, tLeCorrente, tEspera;

int continuar = 1;

void escTensao(void) {
	int h = 0;
	while(continuar) {
		printf("Thread escTensao chama set_medida_tensao com hora %d\n", h);
		set_medida_tensao( 99.99, h++);
		sleep(1);
	}
}


void escCorrente(void) {
	printf("Thread escCorrente vai dormir 5 segundos\n");
	sleep(5);
	printf("Thread escCorrente chama set_medida_corrente com hora %d\n", 16);
	set_medida_corrente( 99.99, 16);
}


double leCorrente( void) {
	double corr;
	printf("Thread leCorrente vair dormir 1 segundo\n");
	sleep(1);
	printf("Thread leCorrente tenta ler corrente\n");
	corr = pega_corrente();
	printf("leCorrente leu corrente %lf\n", corr);
}


void espera(void) {
	printf("Thread espera vai esperar pela sincronizacao dos tempos\n");
	espera_sincronizacao();
	printf("!!! sincronizacao satisfeita !!!\n");
	sleep(1);
	continuar = 0;
}



void main(void) {

	pthread_create( &tEscTensao, NULL, (void *)escTensao, NULL);
	pthread_create( &tEscCorrente, NULL, (void *)escCorrente, NULL);
	pthread_create( &tLeCorrente, NULL, (void *)leCorrente, NULL);
	pthread_create( &tEspera, NULL, (void *)espera, NULL);

	pthread_join( tEscTensao, NULL );
	pthread_join( tEscCorrente, NULL);
	pthread_join( tLeCorrente, NULL);
	pthread_join( tEspera, NULL);
}



