#include	<stdio.h>
#include	<stdlib.h>
#include	<assert.h>
#include	<pthread.h>
#include	<unistd.h>

#include	"solar2022.h"

#include	"planta.h"
#include	"relogio.h"
#include	"console.h"
#include	"instrumentacao.h"

// Passo da simulacao em ms
#define	PASSO_SIMULACAO	1


static pthread_t task_ticker;		// Thread que avança a simulação
static pthread_t task_console;		// Thread para interface via console
static pthread_t task_servidora;	// Thread que recebe comandos via rede para a instrumentação



int solar2022_termina = 0;			// 1 indica que simulação deve terminar


/** Termina o programa em panico
*	s - mensagem
*/
void panico( char *s1, char *s2)
{
	printf("PANICO: %s : %s\n\n", s1, s2);
	exit(-1);
}



/**	Mensagem sobre como usar o programa
*/ 
static void useMsg( char *msg)
{
	if( msg != NULL )
		printf("Erro: %s\n", msg);
		
	printf("use: solar2022 <portaUDP> <enter>\n");
	printf("<portaUDP>: Porta udp para acessar a instrumentacao\n");
	exit(-1);
}



/**	Main
*/
int main( int argc, char *argv[])
{
	printf("------------------------------------------------------\n");
	printf(" SOLAR 2022, simulacao um boiler com aquecimento solar\n");
	printf("------------------------------------------------------\n");
	 
	if( argc != 2 )
		useMsg("Numero de argumentos");

	// Parametros da execução

	int portaBase = atoi( argv[1] );
	if( portaBase < 2048  ||  portaBase > 61535 )
		useMsg("número de porta inválido");

	// Random number generator
	//srand( time(NULL) );
	srand( 555 );
	
	// Cria a ferrovia
	planta_cria();
	printf("Planta criada !!!\n");

	// Inicializa o relogio da simulacao
	printf("Inicializando o relogio ...\n");
	relogio_inicializa(PASSO_SIMULACAO);

	// Inicializa a instrumentação
	printf("Inicializando a instrumentação ...\n");
	instrumentacao_inicializa(portaBase);

	// Cria thread servidora que recebe comandos para a instrumentacao
	printf("Criando thread servidora que recebe comandos para a instrumentação ...\n");
	pthread_create( &task_servidora, NULL, (void *) instrumentacao_threadServidora, NULL);
		
	// Cria thread que avança o estado da planta
	printf("Criando thread da simulação ...\n");
	pthread_create( &task_ticker, NULL, (void *) relogio_threadTicker, NULL);

	// Simulação para em caso de erro
	relogio_pararNoErro( 1 );

	// Inicia a simulação
	printf("Ativando a simulação ...\n");
	relogio_ativaSimulacao();

	// Coloca o console em modo janela
	printf("Inicializando o console ...\n");
	if( console_modoJanela() == -1 )
		exit(-1);
	console_showPlantaTudo();

	// Cria thread para interface via console
	pthread_create( &task_console, NULL, (void *) console_threadConsole, NULL);

	// Espera pelo teclado
	while( solar2022_termina == 0 ) {
		//planta_mostraTudo();
		console_showPlantaTudo();
		sleep(1);
	}

	console_modoNormal();
	console_showTempos();
	printf("Termina o programa\n");
}
			



	

