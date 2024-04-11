#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include <pthread.h>

#include "q9sensores.h"

static pthread_mutex_t em = PTHREAD_MUTEX_INITIALIZER;


static struct dados_medidos_t ultimo_A;
static int tem_ultimo_A = 0;
static pthread_cond_t tem_tipo_A = PTHREAD_COND_INITIALIZER;


static struct dados_medidos_t ultimo_B;
static int tem_ultimo_B = 0;
static pthread_cond_t tem_tipo_B = PTHREAD_COND_INITIALIZER;


static pthread_cond_t alarme = PTHREAD_COND_INITIALIZER;



/* Função auxliar que verifica o alarme
*/
static int aux_situacao_alarme(void) {
	static int vezes = 0;
	++vezes;
	return (vezes >= 10 );
}



/* Permite a uma thread registrar a última medição que ela obteve, podendo 
ser do tipo A ou do tipo B.
*/
void insere_medicao( int tipo, struct dados_medidos_t xxx) {
	pthread_mutex_lock( &em);
	if( tipo == TIPO_A ) {
		ultimo_A = xxx;
		tem_ultimo_A = 1;
		pthread_cond_broadcast( &tem_tipo_A);
	} else {
		ultimo_B = xxx;
		tem_ultimo_B = 1;
		pthread_cond_broadcast( &tem_tipo_B);
	}

	if( aux_situacao_alarme() ) {
		pthread_cond_broadcast( &alarme);
	}

	pthread_mutex_unlock( &em);
}




/* Permite a uma thread ler do monitor a última medição do tipo solicitado.
*/
struct dados_medidos_t consulta_medicao( int tipo) {
	struct dados_medidos_t resposta;

	pthread_mutex_lock( &em);
	if( tipo == TIPO_A ) {
		while( tem_ultimo_A == 0 ) {
			printf("Monitor: consulta_medicao esperando por tipo A\n");
			pthread_cond_wait( &tem_tipo_A, &em);
			resposta = ultimo_A;
		}
	} else {
		while( tem_ultimo_B == 0 ) {
			printf("Monitor: consulta_medicao esperando por tipo B\n");
			pthread_cond_wait( &tem_tipo_B, &em);
			resposta = ultimo_B;
		}
	}
	pthread_mutex_unlock( &em);
	return resposta;
}




/* Bloqueia a thread chamadora até que a condição de alarme torne-se verdadeira.
*/
void espera_alarme(void ) {
	pthread_mutex_lock( &em);

	while( aux_situacao_alarme() == 0 ) {
		printf("Monitor: espera_alarme vai para WAIT\n");
		pthread_cond_wait( &alarme, &em);
	}
	printf("Monitor: espera_alarme saiu do WAIT\n");

	pthread_mutex_unlock( &em);
}




