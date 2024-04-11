#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

pthread_t t1;
pthread_t t2;


void codigo_tarefa_1(void){
	for( int ns=0; ns < 10; ++ns) {
		sleep(2);
		printf("Tarefa 1: passaram %d segundos\n", ns);
        }
}


void codigo_tarefa_2(void){
	for( int ns=0; ns < 15; ++ns) {
		sleep(1);
		printf("Tarefa 2: passaram %d segundos\n", ns);
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



