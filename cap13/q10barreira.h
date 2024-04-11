

#define	N_TRABALHADORES	10



/* thread trabalhadora “meuId” informa que acabou sua parte do serviço
*/
extern void acabei(int meuId);


/* thread finalizadora fica bloqueada até todos os N trabalhadores acabarem o serviço
*/
extern void espera_todos( void );
