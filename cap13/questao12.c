#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "q12buffer.h"

pthread_t produtor[5];
pthread_t consumidor[5];


void main_produtor(int *id) {
	char *s;
	for( int x=0; x<5; ++x) {
		int nc = 1 + rand() % 40;
		char *s = calloc( nc, sizeof(char) );
		printf("Produtor %c vai produzir %d caracteres\n", *id, nc);
		for( int i=0; i<nc; ++i)
			s[i] = *id;
		insere( s, nc);
		sleep(1);
	}
}


void main_consumidor(int *id) {
	char s[N+1];
	while(1) {
		int nc = 1 + rand() % 40;
		retira( s, nc);
		s[nc] = '\0';
		printf("Consumidor %d consumiu %d caracteres>>>%s\n",*id,nc,s);
		sleep(1);
	}
}



void main(void) {
	int idp[] = { 'A','B','C','D','E' };
	int idc[] = { 6,7,8,9,0 };

	for( int i=0; i<5; ++i) {
		pthread_create( &produtor[i], NULL, (void *)main_produtor, &idp[i]);
		pthread_create( &consumidor[i], NULL, (void *)main_consumidor, &idc[i]);
	}

	for( int i=0; i<5; ++i) {
		pthread_join( produtor[i], NULL);
	}
}


