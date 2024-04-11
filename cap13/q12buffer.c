#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "q12buffer.h"


static char buffer[N];


static int prox_ins = 0;	// Próximo ponto de inserção no buffer
static int prox_ret = 0;	// Próximo ponto de remoção do buffer
static int nChar = 0;


static pthread_mutex_t fila_produtor = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t fila_consumidor = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t em = PTHREAD_MUTEX_INITIALIZER;

static pthread_cond_t ter_espaco = PTHREAD_COND_INITIALIZER;
static pthread_cond_t ter_dados = PTHREAD_COND_INITIALIZER;


static void dumpBuffer() {
	char temp[N];
	int p = prox_ret;

	for( int i=0; i<nChar; ++i) {
		temp[i] = buffer[p];
		p = (p+1) % N;
	}
	temp[nChar] = '\0';
	printf("%s\n", temp);
}


/** Insere numero_caracteres caracteres no buffer
*	numero_caracteres < N
*/
void insere( char *s, int numero_caracteres)  {
	pthread_mutex_lock( &fila_produtor);
	pthread_mutex_lock( &em);

	while( N-nChar < numero_caracteres ) {
		pthread_cond_wait( &ter_espaco, &em);
		}

	for( int i=0; i < numero_caracteres; ++i) {
		buffer[prox_ins] = *s++;
		prox_ins = (prox_ins+1) % N;
	}
	nChar = nChar + numero_caracteres;
	pthread_cond_signal( &ter_dados);

	printf("..... inseriu %d caracteres, ocupacao %d\n", numero_caracteres, nChar);
	dumpBuffer();
	pthread_mutex_unlock( &em);
	pthread_mutex_unlock( &fila_produtor);
}



/** Retira numero_caracteres caracteres do buffer
*	numero_caracteres < N
*/
void retira( char *s, int numero_caracteres) {
	pthread_mutex_lock( &fila_consumidor);
	pthread_mutex_lock( &em);

	while( nChar < numero_caracteres )
		pthread_cond_wait( &ter_dados, &em);

	for( int i=0; i < numero_caracteres; ++i) {
		*s++ = buffer[prox_ret];
		prox_ret = (prox_ret+1) % N;
	}
	nChar = nChar - numero_caracteres;
	pthread_cond_signal( &ter_espaco);

	printf("..... retirou %d caracteres, ocupacao %d\n", numero_caracteres, nChar);
	dumpBuffer();

	pthread_mutex_unlock( &em);
	pthread_mutex_unlock( &fila_consumidor);
}



