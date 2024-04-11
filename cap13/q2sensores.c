#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include <pthread.h>

static pthread_mutex_t em = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t tempo_menor = PTHREAD_COND_INITIALIZER;

static pthread_cond_t tem_tensao = PTHREAD_COND_INITIALIZER;
static pthread_cond_t tem_corrente = PTHREAD_COND_INITIALIZER;


static double tensao_ultima = 0;
static long  tensao_hora = LONG_MIN;

static double corrente_ultima = 0;
static long corrente_hora = LONG_MIN;


/* Permite a uma thread registrar a última medição que ela obteve do sensor de tensão e a
hora da medida. Medidas de tensão antigas são descartadas. */
void set_medida_tensao( double tensao, long hora) {
	pthread_mutex_lock( &em);
	printf("Monitor chamado: set_medida_tensao na hora %ld\n", hora);
	tensao_ultima = tensao;
	tensao_hora = hora;
	pthread_cond_broadcast( &tem_tensao);
	pthread_cond_broadcast( &tempo_menor);
	pthread_mutex_unlock( &em);
}

/* Permite a uma thread registrar a última medição que ela obteve do sensor de corrente e
a hora da medida. Medidas de corrente antigas são descartadas. */
void set_medida_corrente( double corrente, long hora) {
	pthread_mutex_lock( &em);
	printf("Monitor chamado: set_medida_corrente na hora %ld\n", hora);
	corrente_ultima = corrente;
	corrente_hora = hora;
	pthread_cond_broadcast( &tem_corrente);
	pthread_cond_broadcast( &tempo_menor);
	pthread_mutex_unlock( &em);
}

/* Permite a uma thread ler do monitor a medição de tensão registrada no monitor. Fica
bloqueada caso nenhuma tensão tenha sido registrada até o momento. */
double pega_tensao( void) {
	double aux;
	pthread_mutex_lock( &em);
	printf("Monitor chamado: pega_tensao\n");
	while( tensao_hora == LONG_MIN ) {
		printf("Monitor: pega_corrente esperando por corrente\n");
		pthread_cond_wait( &tem_tensao, &em);
	}
	aux = tensao_ultima;
	pthread_mutex_unlock( &em);
	return aux;
}

/* Permite a uma thread ler do monitor a medição de corrente registrada no monitor. Fica
bloqueada caso nenhuma corrente tenha sido registrada até o momento. */
double pega_corrente( void) {
	double aux;
	pthread_mutex_lock( &em);
	printf("Monitor chamado: pega_corrente\n");
	while( corrente_hora == LONG_MIN ) {
		printf("Monitor: pega_corrente esperando por corrente\n");
		pthread_cond_wait( &tem_corrente, &em);
	}
	aux = corrente_ultima;
	pthread_mutex_unlock( &em);
	return aux;
}

/* Bloqueia a thread chamadora até que a diferença entre a hora da leitura de tensão e a
hora da leitura de corrente registradas seja menor que 10. */
void espera_sincronizacao( void) {
	pthread_mutex_lock( &em);
	printf("Monitor chamado: espera_sincronizacao\n");
	while( tensao_hora == LONG_MIN	||
			corrente_hora == LONG_MIN	||
			labs(tensao_hora - corrente_hora) >= 10 ) {
		if( tensao_hora != LONG_MIN  &&  corrente_hora != LONG_MIN )
			printf("Diferenca estah em %ld\n", labs(tensao_hora - corrente_hora) );
		pthread_cond_wait( &tempo_menor, &em);
	}
	pthread_mutex_unlock( &em);
}


