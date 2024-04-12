/*	******************
	* RESUMO NCURSES *
	******************

	LINES					// Numero de linhas disponiveis
	COLS					// Numero de colunas disponiveis

	initscr();				// Inicializa o terminal em modo curses

	cbreak();				// Desativa o buffer de linha, CTRL-Z e CTRL-C disparam sinais
	nocbreak();				// Ativa o buffer de linha

	raw();					// Desativa o buffer de linha, CTRL-Z e CTRL-C não disparam sinais
	noraw();				// Ativa o buffer de linha

	echo();					// Teclado ecoa no display
	noecho();				// Teclado nao ecoa no display

	keypad(stdscr, TRUE);	// Permite leitura de teclas de função
	keypad(stdscr, FALSE);	// Teclas de função geram sequencias de escape


	start_color();			// Inicializa as cores
		COLOR_BLACK
		COLOR_RED
		COLOR_GREEN
		COLOR_YELLOW
		COLOR_BLUE
		COLOR_MAGENTA
		COLOR_CYAN
		COLOR_WHITE

	init_pair(n, COLOR_RED, COLOR_BLACK);	// Define par de cores 'n', ordem eh 'foreground','background'


	attron(atribs);			// Ativa atributos
	attroff(atribs);		// Desativa atributos
			A_NORMAL 		// Exibicao normal (sem destaque)
			A_STANDOUT 		// Melhor modo de destaque do terminal
			A_UNDERLINE		// Sublinhado
			A_REVERSE		// Video reverso
			A_BLINK			// Piscando
			A_DIM			// Meio brilhante
			A_BOLD			// Extra brilhante ou negrito
			A_ALTCHARSET 	// Conjunto de caracteres alternativos
			COLOR_PAIR(n)	// Par de cores n que foi pre-definido


	printw("Texto");		// Escreve string nas coordenadas atuais
	printw("%c %d %f", c, d, f);	// Semelhante ao printf

	addch(c);					// Escreve caracter nas coordenadas atuais
	addch(ch | A_BOLD | A_UNDERLINE);


	move(linha,coluna);				// Move o cursor para linha,coluna

	mvprintw(linha,coluna,"Texto");		// Junta o move com o printw
	mvprintw(linha,coluna,"%c %d %f", c, d, f);	// Semelhante ao printf


	refresh();				// Atualiza a tela com as partes modificadas

	getch();				// Le uma tecla (buffer? eco? keypad?)

	getstr(str);			// Le um string

	endwin();				// Encerra o modo curses

*/




/*





	// Testa o conjunto de caracteres alternativos
	move(3,0);				// Move o cursor para inicio da linha 3
	attron(A_ALTCHARSET);
	printw("abcdefghijklmnopqrstuvxz059AM");
	attroff(A_ALTCHARSET);

????????????????????????????????????????????????????????????????????????????????
*/


#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<time.h>
#include	<assert.h>
#include	<pthread.h>
#include	<string.h>
#include 	<ncurses.h>


#include	"console.h"

#include	"solar2022.h"
#include	"relogio.h"
#include	"planta.h"
#include	"instrumentacao.h"


// Linhas na tela
#define LINHA_MENSAGEM	(PLANTA_DADOS+0)
#define LINHA_ENTRADA	(PLANTA_DADOS+1)
#define LINHA_NUVEM		(PLANTA_DADOS+2)


// Pares de cores
#define AMARELO_PRETO		1
#define VERDE_PRETO			2
#define VERMELHO_PRETO		3
#define AZUL_PRETO			4


// Mutex para compartilhar a tela entre várias threads
static pthread_mutex_t tela = PTHREAD_MUTEX_INITIALIZER;



/** Mostra na tela *normal* os tempos da simulação
*/
void console_showTempos(void)
{
	struct EstatisticaSimulacao estats;

	relogio_estatisticas(&estats);
	printf("\nnCiclosSimulados %d\nnDeadlinePerdidos %d\ntickMS %dms\ntotalTempoReal %ldms\ntotalTempoSimulado %ldms\n\n",
		estats.nCiclosSimulados,
		estats.nDeadlinePerdidos,
		estats.tickMS,			
		estats.totalTempoReal,	
		estats.totalTempoSimulado);
}


/** Inicializa o console
* 	Retorna -1 se erro
*/
int console_modoJanela(void)
{
	initscr();				// Inicializa o terminal em modo curses
	start_color();			// Inicializa as cores
	raw();					// Desativa o buffer de linha, CTRL-Z e CTRL-C não disparam sinais
	noecho();				// Teclado nao ecoa no display
	keypad(stdscr, TRUE);	// Permite leitura de teclas de função
	
	if( LINES < PLANTA_DADOS+3 ) {
		console_modoNormal();
		printf("Tamanho (altura) do terminal não é suficiente\n");
		return -1;
	}
	if( COLS < 80 ) {
		console_modoNormal();
		printf("Tamanho (largura) do terminal não é suficiente\n");
		return -1;
	}

	init_pair(AMARELO_PRETO, COLOR_YELLOW, COLOR_BLACK);
	init_pair(VERDE_PRETO, COLOR_GREEN, COLOR_BLACK);
	init_pair(VERMELHO_PRETO, COLOR_RED, COLOR_BLACK);
	init_pair(AZUL_PRETO, COLOR_BLUE, COLOR_BLACK);

	return 0;	
}


/** Retorna o terminal ao modo normal
*/
void console_modoNormal(void)
{
	keypad(stdscr, FALSE);	// Retorna ao default
	noraw();				// Retorna ao default
	echo();					// Retorna ao default
	endwin();				// Encerra o modo curses
}


/** Move o cursor para posição de descanso
*	Não obtem mutex da tela
*/
static void descansaCursor(void)
{
	move(LINHA_ENTRADA,59);
}


/** Coloca mensagem na tela
*	Não obtem mutex da tela
*/
static void mensagem(char *s)
{
	move(LINHA_MENSAGEM,0);
	printw(s);	
}


/** Atualiza o terminal com os dados da planta, em modo janela
*/
void console_showPlantaTudo(void)
{
	char **textos = planta_textosCompletos();
	double dados[PLANTA_DADOS];
	int atrib;

	planta_dadosCompletos(dados);

	pthread_mutex_lock(&tela);

	// Linhas apenas para leitura
	for( int i=0; i<PLANTA_DADOS-1-5-1; ++i) {
		if( i%2==0 )
			atrib = A_NORMAL;
		else
			atrib = COLOR_PAIR(AMARELO_PRETO);
		attron(atrib);
		mvprintw(i,0,"%s: %8.3lf", textos[i], dados[i]);
		attroff(atrib);		// Desativa atributos
	}

	// Linha para +/-
	if( PLANTA_DADOS-1-5-1%2==0 )
		atrib = A_NORMAL;
	else
		atrib = COLOR_PAIR(AMARELO_PRETO);
	attron(atrib);
	mvprintw(PLANTA_DADOS-1-5-1,0,"%s: %8.3lf      [+|-]", textos[PLANTA_DADOS-1-5-1], dados[PLANTA_DADOS-1-5-1]);
	attroff(atrib);		// Desativa atributos


	// Linhas para liga/desliga
	for( int i=PLANTA_DADOS-5-1; i<PLANTA_DADOS-1; ++i) {
		if( dados[i] != 0 )
			atrib = COLOR_PAIR(VERDE_PRETO);
		else
			atrib = COLOR_PAIR(VERMELHO_PRETO);
		attron(atrib);
		mvprintw(i,0,"%s:  %3d    [F%d]", textos[i], (int)dados[i], i-PLANTA_DADOS+7);
		attroff(atrib);		// Desativa atributos
	}

	// Linha de erro
	if( dados[PLANTA_DADOS-1] != 0 )
		atrib = COLOR_PAIR(VERMELHO_PRETO) | A_BLINK;
	else
		atrib = COLOR_PAIR(VERDE_PRETO);
	attron(atrib);
	mvprintw(PLANTA_DADOS-1, 0, "%s:   %s", textos[PLANTA_DADOS-1], dados[PLANTA_DADOS-1]!=0?"ERRO":"OK");
	attroff(atrib);		// Desativa atributos

	if( planta_leTempBoiler() > 100 )
		mensagem("leTempBoiler() > 100");

	if( dados[13] > 100 )
		mensagem("dados[13] > 100");

	descansaCursor();
	refresh();
	pthread_mutex_unlock(&tela);
}


/** Código da thread para interface via console
*/
void console_threadConsole(void)
{
	int c;
	double v;
	int referencia;
	char mensagem[100];
	
	pthread_mutex_lock(&tela);
	move(LINHA_ENTRADA,0);
	printw("Digite n + - F1 F2 F3 F4 F5  p/comandos  ou  X p/terminar: ");
	move(LINHA_NUVEM,0);
	printw(".....");
	refresh();
	pthread_mutex_unlock(&tela);

	do {
		c = getch();		// Espera por uma tecla
		switch(c) {
			case 'n':
					referencia = 30 + 10 * (rand()%4);		// Entre 30 e 60
					sprintf( mensagem, "%d", referencia);	
					pthread_mutex_lock(&tela);
					move(LINHA_NUVEM,0);
					printw("ENVIADO %d", referencia);
					refresh();
					pthread_mutex_unlock(&tela);
					instrumentacao_enviaControlador(mensagem);
					console_showPlantaTudo();
					break;
			case '+':
					v = planta_leVazaoConsumo();
					planta_defineVazaoConsumo( v + 0.1 );
					console_showPlantaTudo();
					break;
			case '-':
					v = planta_leVazaoConsumo();
					planta_defineVazaoConsumo( v - 0.1 );
					console_showPlantaTudo();
					break;
			case KEY_F(1):
					planta_acionaBombaColetor( (planta_leBombaColetor()+1) % 2 );
					console_showPlantaTudo();
					break;
			case KEY_F(2):
					planta_acionaBombaCirculacao( (planta_leBombaCirculacao()+1) % 2 );
					console_showPlantaTudo();
					break;
			case KEY_F(3):
					planta_acionaAquecedor( (planta_leAquecedor()+1) % 2 );
					console_showPlantaTudo();
					break;
			case KEY_F(4):
					planta_acionaValvulaEntrada( (planta_leValvulaEntrada()+1) % 2);
					console_showPlantaTudo();
					break;
			case KEY_F(5):
					planta_acionaValvulaEsgoto( (planta_leValvulaEsgoto()+1) % 2 );
					console_showPlantaTudo();
					break;
			case 'X':
			case 'x':
					pthread_mutex_lock(&tela);
					printw("TERMINAR !!!");
					refresh();
					pthread_mutex_unlock(&tela);
					solar2022_termina = 1;
					break;
			default:
					break;
		}
	} while( solar2022_termina == 0 );
	

}


