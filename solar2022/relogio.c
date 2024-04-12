#include	<stdio.h>
#include	<time.h>
#include	<assert.h>
#include	<pthread.h>

#include	"relogio.h"

#include	"solar2022.h"
#include	"planta.h"
#include	"evento.h"


static pthread_mutex_t em = PTHREAD_MUTEX_INITIALIZER;

static pthread_cond_t okSimular = PTHREAD_COND_INITIALIZER;



static int tickMS;						// Atualiza o estado da simulacao a cada tickMS (miliseconds)

static int simula = 0;					// 1=simula		0=nao simula

static int paraNoErro = 0;				// Para a simulação em caso de erro

static int nDeadlinePerdidos = 0;		// Numero de deadlines (ciclos) perdidos
	
static int nCiclosSimulados = 0;		// Número total de ciclos simulados

static long totalTempoSimulado = 0;		// Tempo total simulado em milisegundos

static struct timespec inicioSimulacao;	// Inicio da simulacao em tempo real



/** Inicializa relogio da simulacao
*	tick - duracao de um tick da simulacao em milisegundos
*/
void relogio_inicializa( int tick)
{
	if( tick < 1  ||  tick > 100 )
		panico("relogio_inicializa: tick fora dos limites 1 e 100","");

	pthread_mutex_lock( &em);
	assert( simula == 0 );
	tickMS = tick;
	simula =  0;
	nDeadlinePerdidos = 0;
	nCiclosSimulados = 0;
	totalTempoSimulado = 0;
	pthread_mutex_unlock( &em);
}



/** Define a duração do passo de simulação em milisegundos
	tick - passo da simulação em milisegundos
*/
void relogio_setTick( int tick)
{
	if( tick < 1  ||  tick > 100 )
		return;

	pthread_mutex_lock( &em);
	tickMS = tick;
	pthread_mutex_unlock( &em);
}


/** Obtem a duração do passo de simulação em milisegundos
*/
int relogio_getTick(void)
{
	int t;
	pthread_mutex_lock( &em);	
	t = tickMS;
	pthread_mutex_unlock( &em);
	return t;
}


/**	Informa se deve parar a simulação em caso de erro
*/
int relogio_leParaNoErro(void)
{
	return paraNoErro;
}


/**	Define se deve parar simulacao em caso de erro
*	parar - 1 indica que deve parar
*/
void relogio_pararNoErro( int parar)
{
	paraNoErro = parar;
}


/**	Retorna o número de deadlines perdidos e total de passos simulados
*/
void relogio_getDuracao( int *ndp, int *ncs)
{
	pthread_mutex_lock( &em);
	*ndp = nDeadlinePerdidos;
	*ncs = nCiclosSimulados;
	pthread_mutex_unlock( &em);
}


/** Ativa a simulação
*/
void relogio_ativaSimulacao(void)
{
	pthread_mutex_lock( &em);
	simula = 1;
	pthread_cond_signal( &okSimular);
	pthread_mutex_unlock( &em);
}
	

/** Pausa a simulação
*/
void relogio_pausaSimulacao(void)
{
	pthread_mutex_lock( &em);
	simula = 0;
	pthread_mutex_unlock( &em);
}


/** Retorna informação sobre simulação ativada (1) ou pausada (0)
*/
int relogio_leEstadoSimulacao(void)
{
	int s;	
	pthread_mutex_lock( &em);
	s = simula;
	pthread_mutex_unlock( &em);
	return s;
}


/**	Retorna estatisticas da simulação
*/
struct EstatisticaSimulacao *relogio_estatisticas( struct EstatisticaSimulacao *es)
{
	struct timespec agora;

	pthread_mutex_lock( &em);
	es->nCiclosSimulados = nCiclosSimulados;
	es->nDeadlinePerdidos = nDeadlinePerdidos;
	es->tickMS = tickMS;

	clock_gettime(CLOCK_MONOTONIC, &agora);
	es->totalTempoReal = ((agora.tv_sec-inicioSimulacao.tv_sec)*1000) +
						 ((agora.tv_nsec-inicioSimulacao.tv_nsec)/1000000);

	es->totalTempoSimulado = totalTempoSimulado;
	pthread_mutex_unlock( &em);

	return es;
}


/** Código da thread que faz avançar o relógio da simulação
*/
void relogio_threadTicker(void)
{
	struct timespec t, t1;
	int tempoResposta;			// microsegundos

	printf("relogio_threadTicker criada ...\n");

	// Sempre espera 1 segundo para iniciar
	clock_gettime(CLOCK_MONOTONIC ,&t);
	t.tv_sec++;
	inicioSimulacao = t;

	while( solar2022_termina == 0 ) {
		int passo;		// milisegundos

		// Confere se simulacao está autorizada e qual o passo
		pthread_mutex_lock( &em);
		while( simula == 0 ) {
			printf("relogio_threadTicker esperando ...\n");
			pthread_cond_wait( &okSimular, &em);
			// Precisa resincronizar com o tempo real
			clock_gettime(CLOCK_MONOTONIC ,&t);	
			t.tv_sec++;
			}
		passo = tickMS;
		pthread_mutex_unlock( &em);

		// Calcula instante do próximo passo
		t.tv_nsec += passo * 1000000;		// milisegundos para nanosegundos
		while (t.tv_nsec >= NSEC_PER_SEC) {
			t.tv_nsec -= NSEC_PER_SEC;
			t.tv_sec++;
			}

		// Espera até o instante correto
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);

		// Passo da simulação
		totalTempoSimulado += passo;			// Avança o relógio
		planta_tickUpdate(passo);				// Calcula novo estado da planta
		evento_processa(totalTempoSimulado);	// Processa eventos armados para este momento

		// Recolhe estatisticas
		++nCiclosSimulados;
		clock_gettime(CLOCK_MONOTONIC ,&t1);
		tempoResposta = (t1.tv_sec - t.tv_sec) * 1000000;	// microsegundos
		tempoResposta += (t1.tv_nsec - t.tv_nsec) / 1000;	// microsegundos

		if( tempoResposta > passo*1000 )
			++nDeadlinePerdidos;
	
		//if(nSimulatedCycles%103==0)  possibilidade
		//	printf("%d\n", responseTime);

		// Talvez pare simulação se erro foi detectado
		if( planta_leErroDetectado() && paraNoErro )
			return;		// Mata a thread de simulacao
		}
}


/** Retorna o tempo simulado em milisegundos
*/
long relogio_leTempoSimulado()
{
	return totalTempoSimulado;
}




