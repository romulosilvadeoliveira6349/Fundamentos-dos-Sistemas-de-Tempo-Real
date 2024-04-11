
/* Permite a uma thread registrar a última medição que ela obteve do sensor de tensão e a
hora da medida. Medidas de tensão antigas são descartadas. */

extern void set_medida_tensao( double tensao, long hora);

/* Permite a uma thread registrar a última medição que ela obteve do sensor de corrente e
a hora da medida. Medidas de corrente antigas são descartadas. */

extern void set_medida_corrente( double corrente, long hora);

/* Permite a uma thread ler do monitor a medição de tensão registrada no monitor. Fica
bloqueada caso nenhuma tensão tenha sido registrada até o momento. */

extern double pega_tensao( void);

/* Permite a uma thread ler do monitor a medição de corrente registrada no monitor. Fica
bloqueada caso nenhuma corrente tenha sido registrada até o momento. */

extern double pega_corrente( void);

/* Bloqueia a thread chamadora até que a diferença entre a hora da leitura de tensão e a
hora da leitura de corrente registradas seja menor que 10. */

extern void espera_sincronizacao( void);


