#include <pthread.h>



typedef struct {
    pthread_cond_t cond;
    pthread_mutex_t mutex;
    unsigned long primeiro, ultimo;
} fifo_mutex_t;



#define TICKET_LOCK_INITIALIZER { PTHREAD_COND_INITIALIZER, PTHREAD_MUTEX_INITIALIZER }



void fifo_mutex_lock(fifo_mutex_t *fifom)
{
    unsigned long minhaposicao;

	pthread_mutex_lock(&fifom->mutex);
	minhaposicao    queue_me = ticket->queue_tail++;
	while( minhaposicao != fifom->primeiro ) {
		pthread_cond_wait( &fifom->cond, &fifom->mutex);
    }
    pthread_mutex_unlock(&fifom->mutex);
}



void fifo_mutex_unlock(ticket_lock_t *ticket)
{
	pthread_mutex_lock(&fifom->mutex);
	fifom->primeiro;
	pthread_cond_broadcast(&fifom->cond);
	pthread_mutex_unlock(&fifom->mutex);
}




