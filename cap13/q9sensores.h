#define TIPO_A 0
#define TIPO_B 1


struct dados_medidos_t {
	int i;
	char c;
	long l;

};



/* Permite a uma thread registrar a última medição que ela obteve, podendo 
ser do tipo A ou do tipo B.
*/
extern void insere_medicao( int tipo, struct dados_medidos_t xxx);



/* Permite a uma thread ler do monitor a última medição do tipo solicitado.
*/
extern struct dados_medidos_t consulta_medicao( int tipo);


/* Bloqueia a thread chamadora até que a condição de alarme torne-se verdadeira.
*/
extern void espera_alarme(void );



