#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>

static pthread_mutex_t em = PTHREAD_MUTEX_INITIALIZER;

static pthread_cond_t saldo_suficiente = PTHREAD_COND_INITIALIZER;
static pthread_cond_t passou_valor_vip = PTHREAD_COND_INITIALIZER;
static pthread_cond_t pode_retirar = PTHREAD_COND_INITIALIZER;


static double saldo = 0;
static double soma_depositos = 0;
static int retirada_em_andamento = 0;



/* Deposita um valor na conta, soma no saldo, nunca bloqueia. */

void deposita( double valor) {
	pthread_mutex_lock( &em);
	printf("Monitor chamado: deposita %lf\n", valor);
	saldo += valor;
	pthread_cond_signal( &saldo_suficiente);
	soma_depositos += valor;
	pthread_cond_broadcast( &passou_valor_vip);
	pthread_mutex_unlock( &em);
}


/* Retira o valor da conta, subtrai do saldo, fica bloqueado até que
exista saldo suficiente na conta para atender a retirada. Pedidos de
retirada são atendidos por ordem de chegada. */

void retira( double valor) {
	pthread_mutex_lock( &em);
	printf("Monitor chamado: retira %lf\n", valor);

	while( retirada_em_andamento )
		pthread_cond_wait( &pode_retirar, &em);


	retirada_em_andamento = 1;
	while( saldo < valor )
		pthread_cond_wait( &saldo_suficiente, &em);

	saldo -= valor;
	retirada_em_andamento = 0;

	pthread_cond_signal( &pode_retirar);
	pthread_mutex_unlock( &em);
}


/* Bloqueia a thread até que o somatório dos depósitos, sem
considerar as retiradas, ultrapasse o valor_vip. */

void espera_saldo_vip( double valor_vip) {
	pthread_mutex_lock( &em);
	printf("Monitor chamado: espera_saldo_vip %lf\n", valor_vip);

	while( soma_depositos < valor_vip )
		pthread_cond_wait( &passou_valor_vip, &em);

	pthread_mutex_unlock( &em);
}


/* Aplica sobre o saldo da conta uma correção monetária, fornecida
em pontos percentuais.*/

void aplica_correcao( double correcaoPP) {
	pthread_mutex_lock( &em);
	printf("Monitor chamado: aplica_correcao %lf\n", correcaoPP);
	saldo = saldo * correcaoPP;
	pthread_cond_signal( &saldo_suficiente);
	pthread_mutex_unlock( &em);
}




