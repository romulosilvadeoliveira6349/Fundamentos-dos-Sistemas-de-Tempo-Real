#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "q3conta.h"


pthread_t tDeposita;
pthread_t tRetira2000, tRetira1000, tRetira500;
pthread_t tEsperaVip, tCorrecao;


void main_deposita(void) {
	for( int i=0; i<40; ++i) {
		deposita(100);
		sleep(1);
	}
}


void main_retira2000(void) {
	sleep(1);
	retira(2000);
	printf("main_retira2000: Retirou 2000\n");
}


void main_retira1000(void) {
	sleep(3);
	retira(1000);
	printf("main_retira1000: Retirou 1000\n");
}


void main_retira500(void) {
	sleep(5);
	retira(500);
	printf("main_retira500: Retirou 500\n");
}


void main_esperaVip(void) {
	printf("main_esperaVip: Estou esperando ficar vip\n");
	espera_saldo_vip(900);
	printf("main_esperaVip: Fiquei vip !!!\n");
}


void main_correcao(void) {
	sleep(21);
	printf("main_correcao: Aplica correcao\n");
	aplica_correcao(2.00);
}


void main(void) {

	pthread_create( &tDeposita, NULL, (void *)main_deposita, NULL);
	pthread_create( &tRetira2000, NULL, (void *)main_retira2000, NULL);
	pthread_create( &tRetira1000, NULL, (void *)main_retira1000, NULL);
	pthread_create( &tRetira500, NULL, (void *)main_retira500, NULL);
	pthread_create( &tEsperaVip, NULL, (void *)main_esperaVip, NULL);
	pthread_create( &tCorrecao, NULL, (void *)main_correcao, NULL);

	pthread_join( tDeposita, NULL);
	pthread_join( tRetira2000, NULL);
	pthread_join( tRetira1000, NULL);
	pthread_join( tRetira500, NULL);
	pthread_join( tEsperaVip, NULL);
	pthread_join( tCorrecao, NULL);
}



