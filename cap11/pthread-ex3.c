#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

pthread_t t1;
pthread_t t2;

char historico[2000];
pthread_mutex_t mutex_historico = PTHREAD_MUTEX_INITIALIZER;


void codigo_tarefa_1(void){
	for( int ns=0; ns < 10; ++ns) {
		sleep(1);
		printf("Tarefa 1: passaram %d segundos\n", ns);

 		pthread_mutex_lock(&mutex_historico);
  		strcat( historico, "Tarefa 1 ---");

 		if( ns == 5 )	getchar();

 		pthread_mutex_unlock(&mutex_historico);

 	}
}


void codigo_tarefa_2(void){
	for( int ns=0; ns < 15; ++ns) {
		sleep(1);
		printf("Tarefa 2: passaram %d segundos\n", ns);

 		pthread_mutex_lock(&mutex_historico);
		strcat( historico, "Tarefa 2 ---");
 		pthread_mutex_unlock(&mutex_historico);
	}
}


void main(){
	printf("Inicio\n");

	pthread_create(&t1, NULL, (void *) codigo_tarefa_1, NULL);

	pthread_create(&t2, NULL, (void *) codigo_tarefa_2, NULL);

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);

	printf("%s\n", historico);
	printf("Fim\n");
}



