#include	<stdio.h>
#include	<stdlib.h>

#include	"evento.h"

#include	"relogio.h"
#include	"planta.h"
#include	"solar2022.h"


static struct Evento *livres = NULL;

static struct Evento *futuros = NULL;


/** Libera um evento
*	evento - evento a ser liberado
*/
void evento_liberaUm( struct Evento *evento)
{
	evento->prox = livres;
	livres = evento;
}


/** Aloca uma estrutura tipo Evento
*/
struct Evento *evento_aloca(void)
{
	struct Evento *novo;
	if( livres != NULL ) {
		novo = livres;
		livres = livres->prox;
	}
	else {
		novo = malloc( sizeof(struct Evento) );	
	}
	return novo;
}


/** Insere em uma lista ordenada pelo campo instanteFuturo
*	lista - lista de eventos ordenada
*	novo - novo evento a ser inserido na lista
*	Retorna o novo primeiro da lista
*/
struct Evento *evento_insere( struct Evento *lista, struct Evento *novo)
{
	struct Evento *x, *ant;

	if( lista == NULL )		// sera o unico
	  {
		novo->prox = NULL;
		novo->ant = NULL;
		lista = novo;
		return lista;
		}

	ant = NULL;
	x = lista;
	while( x != NULL  &&  novo->instanteFuturo >= x->instanteFuturo ) {
		ant = x;
		x = x->prox;
	}	

	if( x == NULL )				// sera o ultimo
	  {
		novo->prox = NULL;
		novo->ant = ant;
		novo->ant->prox = novo;
		}
	else if( x == lista )		// sera o primeiro
	  {
		novo->prox = lista;
		novo->ant = NULL;		
		lista = novo;
		novo->prox->ant = novo;
		}
	else 						// fica no meio
	  {
		novo->prox = x;
		novo->ant = x->ant;		
		x->ant = novo;
		novo->ant->prox = novo;
		}

	return lista;
}



/** Retira de uma lista ordenada pelo campo instanteFuturo
*	lista - lista de eventos ordenada
*	velho - evento a ser retirado da lista
*	Retorna o novo primeiro da lista
*/
struct Evento *evento_retira( struct Evento *lista, struct Evento *velho)
{
	struct Evento *x;

	x = lista;
	while( x != NULL  &&  x != velho )
		x = x->prox;

	if( x == NULL )
		panico("evento_retira: deve retirar elemento que não está na lista","");
		
	if( x->ant == NULL )
		lista = x->prox;
	else
		x->ant->prox = x->prox;

	if( x->prox != NULL )
		x->prox->ant = x->ant;

	evento_liberaUm(x);

	return lista;
}



/** Mostra a lista de eventos ordenada pelo campo instanteFuturo
*	lista - lista de eventos ordenada
*/
void evento_showLista( struct Evento *lista)
{
	struct Evento *x = lista;
	
	if( lista == NULL )
		printf("LISTA DE EVENTOS VAZIA\n");
	else
		printf("LISTA DE EVENTOS\n");

	while( x != NULL ) {
		printf("Evento tipo %d em %ld\n", x->tipo, x->instanteFuturo);
		x = x->prox;
	}
}



/** Cria um evento novo e insere na lista de eventos
*	tipo - tipo do evento
*	instante - quando o evento ocorrerá no futuro
*/
void evento_cria( int tipo, long instante)
{
	struct Evento *evento = evento_aloca();
	
	evento->tipo = tipo;
	evento->instanteFuturo = instante;
	
	futuros = evento_insere( futuros, evento);
}



/** Processa os eventos no instante correto
*	agora - instante atual
*/
void evento_processa( long agora)
{
	struct Evento *evento;
	
	while( futuros != NULL  &&   futuros->instanteFuturo <= agora ) {
		// Retira o primeiro evento da lista
		evento = futuros;
		futuros = futuros->prox;
		if( futuros != NULL )
			futuros->ant = NULL;
		
		// Vários tipos de eventos
		switch( evento->tipo ) {
			case evento_FAZ_NADA:
				evento_liberaUm(evento);
				break;
			case evento_MUDA_CONSUMO:
//				planta_mudaConsumo( evento->???);
				evento_liberaUm(evento);
				break;
			default:
				panico("evento_processa: evento com tipo desconhecido", "");		
		}
	}	
}





