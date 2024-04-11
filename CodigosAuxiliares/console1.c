#include	<stdio.h>
#include	<stdlib.h>
#include	<time.h>
#include	<pthread.h>
#include	<string.h>

#include	"console1.h"



static pthread_mutex_t tela = PTHREAD_MUTEX_INITIALIZER;





/** Aloca a tela
*/
void console_pegaTela(void) {
	pthread_mutex_lock( &tela);
}



/** Libera a tela
*/
void console_largaTela(void) {
	pthread_mutex_unlock( &tela);
}





