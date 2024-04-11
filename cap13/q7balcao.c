#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <pthread.h>

struct proposta_t {
	double valor;
	double resultado;
};


static pthread_mutex_t em = PTHREAD_MUTEX_INITIALIZER;

static pthread_cond_t alteraC = PTHREAD_COND_INITIALIZER;
static pthread_cond_t alteraV = PTHREAD_COND_INITIALIZER;

static struct proposta_t *melhor_v = NULL;
static struct proposta_t *melhor_c = NULL;


static void analisaCompra( struct proposta_t *p)
{
	// Verifica se passou a ser a melhor
	if( melhor_c == NULL ) {
		printf("Assume oferta compra %lf como melhor\n", p->valor);
		melhor_c = p;
		p->resultado = 0;
	} else if( melhor_c != NULL  &&  p->valor > melhor_c->valor ) {
		printf("Assume oferta compra %lf como melhor no lugar de %lf\n", p->valor, melhor_c->valor);
		melhor_c->resultado = -1;
		melhor_c = p;
		p->resultado = 0;
		pthread_cond_signal( &alteraC);
	} else {
		printf("Descarta oferta compra %lf pois melhor eh %lf\n", p->valor, melhor_c->valor);
		p->resultado = -1;
	}
	
	// Verifica se deu negocio
	if( p->resultado == 0  &&
		melhor_v != NULL  &&  p->valor >= melhor_v->valor ) {
		double negocio = ( p->valor + melhor_v->valor ) / 2;
		printf("Fechado negocio com compra %lf e venda %lf == %lf\n", p->valor, melhor_v->valor, negocio);
		p->resultado = negocio;
		melhor_v->resultado = negocio;
		melhor_v = NULL;
		melhor_c = NULL;
		pthread_cond_signal( &alteraC);		
		pthread_cond_signal( &alteraV);		
	}
}


static void analisaVenda( struct proposta_t *p)
{
	// Verifica se passou a ser a melhor
	if( melhor_v == NULL ) {
		printf("Assume oferta venda %lf como melhor\n", p->valor);
		melhor_v = p;
		p->resultado = 0;
	} else if( melhor_v != NULL  &&  p->valor < melhor_v->valor ) {
		printf("Assume oferta venda %lf como melhor no lugar de %lf\n", p->valor, melhor_v->valor);
		melhor_v->resultado = -1;
		melhor_v = p;
		p->resultado = 0;
		pthread_cond_signal( &alteraV);
	} else {
		printf("Descarta oferta venda %lf pois melhor eh %lf\n", p->valor, melhor_v->valor);
		p->resultado = -1;
	}
	
	// Verifica se deu negocio
	if( p->resultado == 0  &&
		melhor_c != NULL  &&  p->valor <= melhor_c->valor ) {
		double negocio = ( p->valor + melhor_c->valor ) / 2;
		printf("Fechado negocio com compra %lf e venda %lf == %lf\n", melhor_c->valor, p->valor, negocio);
		p->resultado = negocio;
		melhor_c->resultado = negocio;
		melhor_v = NULL;
		melhor_c = NULL;
		pthread_cond_signal( &alteraC);		
		pthread_cond_signal( &alteraV);		
	}
}





double ofertaCompra( double valor)
{
	struct proposta_t pc;
	pthread_mutex_lock( &em);
	printf("Monitor chamado: ofertaCompra %lf\n", valor);
	pc.valor = valor;
	pc.resultado = 0;

	analisaCompra( &pc);
	while( pc.resultado == 0 )
		pthread_cond_wait( &alteraC, &em);

	pthread_mutex_unlock( &em);
	return pc.resultado;
}



double ofertaVenda( double valor)
{
	struct proposta_t pv;
	pthread_mutex_lock( &em);
	printf("Monitor chamado: ofertaVenda %lf\n", valor);
	pv.valor = valor;
	pv.resultado = 0;

	analisaVenda( &pv);
	while( pv.resultado == 0 )
		pthread_cond_wait( &alteraV, &em);

	pthread_mutex_unlock( &em);
	return pv.resultado;
}



