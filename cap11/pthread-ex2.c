#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

pthread_t t1;
pthread_t t2;


struct param_t {
	int ns;
	char *nome;
};
	

void codigo_tarefa(struct param_t *pparam){
	for( int ns=0; ns < pparam->ns; ++ns) {
		sleep(1);
		printf("%s: passaram %d segundos\n", pparam->nome, ns);
	}
}



void main(){

	struct param_t p1, p2;
	
	p1.ns = 5;
	p1.nome = "TAREFA 1"; 	
	
	p2.ns = 10;
	p2.nome = "TAREFA 2"; 	

	printf("Inicio\n");

	pthread_create(&t1, NULL, (void *) codigo_tarefa, (void *) &p1);

	pthread_create(&t2, NULL, (void *) codigo_tarefa, (void *) &p2);

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);

	printf("Fim\n");
}





