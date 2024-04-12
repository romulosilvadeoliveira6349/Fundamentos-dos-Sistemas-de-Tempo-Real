#ifndef		EVENTO_H
#define		EVENTO_H


#define	evento_FAZ_NADA			901
#define	evento_MUDA_CONSUMO		902




struct Evento {
	int tipo;
	long instanteFuturo;
	struct Evento *prox;
	struct Evento *ant;
};



/** Libera um evento
*	evento - evento a ser liberado
*/
void evento_liberaUm( struct Evento *evento);


/** Aloca uma estrutura tipo Evento
*/
struct Evento *evento_aloca(void);


/** Insere em uma lista ordenada pelo campo instanteFuturo
*	lista - lista de eventos ordenada
*	evento - novo evento a ser inserido na lista
*	Retorna o novo primeiro da lista
*/
struct Evento *evento_insere( struct Evento *lista, struct Evento *evento);


/** Retira de uma lista ordenada pelo campo instanteFuturo
*	lista - lista de eventos ordenada
*	velho - evento a ser retirado da lista
*	Retorna o novo primeiro da lista
*/
struct Evento *evento_retira( struct Evento *lista, struct Evento *velho);


/** Mostra a lista de eventos ordenada pelo campo instanteFuturo
*	lista - lista de eventos ordenada
*/
void evento_showLista( struct Evento *lista);


/** Cria um evento novo e insere na lista de eventos
*	tipo - tipo do evento
*	instante - quando o evento ocorrerá no futuro
*/
void evento_cria( int tipo, long instante);


/** Processa os eventos no instante correto
*	agora - instante atual
*/
void evento_processa( long agora);






#endif		// EVENTO





