#ifndef		RELOGIO_H
#define		RELOGIO_H


struct EstatisticaSimulacao {
	int nCiclosSimulados;		// Total de ciclos simulados
	int nDeadlinePerdidos;		// Numero de deadlines perdidos
	int tickMS;					// Duracao do tick em milisegundos
	long totalTempoReal;		// Tempo total em tempo real (milisegundos)
	long totalTempoSimulado;	// Tempo total simulado (milisegundos)
};





/** Inicializa relogio da simulacao
*	tick - duracao de um tick da simulacao em milisegundos
*/
void relogio_inicializa( int tick);


/** Define a duração do passo de simulação em milisegundos
	tick - passo da simulação em milisegundos
*/
void relogio_setTick( int tick);


/** Obtem a duração do passo de simulação em milisegundos
*/
int relogio_getTick(void);


/**	Informa se deve parar a simulação em caso de erro
*/
int relogio_leParaNoErro(void);


/**	Define se deve parar simulacao em caso de erro
*	parar - 1 indica que deve parar
*/
void relogio_pararNoErro( int parar);


/**	Retorna o número de deadlines perdidos e total de passos simulados
*/
void relogio_getDuracao( int *ndp, int *ncs);


/** Ativa a simulação
*/
void relogio_ativaSimulacao(void);
	

/** Pausa a simulação
*/
void relogio_pausaSimulacao(void);


/** Retorna informação sobre simulação ativada (1) ou pausada (0)
*/
int relogio_leEstadoSimulacao(void);


/**	Retorna estatisticas da simulação
*/
struct EstatisticaSimulacao *relogio_estatisticas( struct EstatisticaSimulacao *es);


/** Código da thread que faz avançar o relógio da simulação
*/
void relogio_threadTicker(void);


/** Retorna o tempo simulado em milisegundos
*/
long relogio_leTempoSimulado();



#endif		// RELOGIO_H







