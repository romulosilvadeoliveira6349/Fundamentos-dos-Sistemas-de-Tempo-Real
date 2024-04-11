#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include <pthread.h>

#include "q10barreira.h"

static pthread_mutex_t em = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t todosTerminaram = PTHREAD_COND_INITIALIZER;

static int terminados = 0;




/* thread trabalhadora “meuId” informa que acabou sua parte do serviço
*/
void acabei (int meuId) {
	pthread_mutex_lock( &em);
	++terminados;
	if( terminados >= N_TRABALHADORES )
		pthread_cond_signal( &todosTerminaram);
	pthread_mutex_unlock( &em);
}





/* thread finalizadora fica bloqueada até todos os N trabalhadores acabarem o serviço
*/
void espera_todos( void ) {
	pthread_mutex_lock( &em);
	while( terminados < N_TRABALHADORES ) {
		printf("Monitor: espera_todos vai executar WAIT\n");
		pthread_cond_wait( &todosTerminaram, &em);
	}
	printf("Monitor: espera_todos liberada\n");

	//terminados -= N_TRABALHADORES;

	pthread_mutex_unlock( &em);
}





