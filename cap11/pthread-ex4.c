#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

pthread_t t1;
pthread_t t2;

pthread_mutex_t em = PTHREAD_MUTEX_INITIALIZER;


void codigo_tarefa_1(void){
	for( int ns=0; ns < 15; ++ns) {
		//sleep(1);
		printf("Tarefa 1: passaram %d segundos\n", ns);

 		pthread_mutex_lock(&em);
		printf("Tarefa 1: estah com o Mutex\n");
		 sleep(1);	// Errado !!!
 		pthread_mutex_unlock(&em);
 		sched_yield();  //
 	}
}


void codigo_tarefa_2(void){
	for( int ns=0; ns < 15; ++ns) {
		sleep(1);
		printf("Tarefa 2: passaram %d segundos\n", ns);

 		pthread_mutex_lock(&em);
		printf("Tarefa 2: estah com o Mutex\n");
 		pthread_mutex_unlock(&em);
	}
}


void main(){
	printf("Inicio\n");

	pthread_create(&t1, NULL, (void *) codigo_tarefa_1, NULL);

	pthread_create(&t2, NULL, (void *) codigo_tarefa_2, NULL);

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);

	printf("Fim\n");
}



