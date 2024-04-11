/**
	Sugestoes de leitura:

	https://terminalroot.com.br/ncurses/

	https://tldp.org/HOWTO/NCURSES-Programming-HOWTO/




	**********
	* RESUMO *
	**********

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


#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>



int main(){
	int c;

	initscr();			// Inicializa o terminal em modo curses
	start_color();		// Inicializa as cores
	raw();				// Desativa o buffer de linha, CTRL-Z e CTRL-C não disparam sinais
	noecho();			// Teclado nao ecoa no display
	keypad(stdscr, TRUE);		// Permite leitura de teclas de função

	printw("Escreve na posicao 0,0");	// Escreve nas coordenadas atuais (0,0)
	refresh();				// Atualiza a tela com as partes modificadas


	printw("   Digite F1 ou F2 ou F3: ");	// Escreve nas coordenadas atuais
	do {
		c = getch();		// Espera por uma tecla
		attron(A_BLINK);
		switch(c) {
			case KEY_F(1):	printw("F1 foi pressionada");
					break;
			case KEY_F(2):	printw("F2 foi pressionada");
					break;
			case KEY_F(3):	printw("F3 foi pressionada");
					break;
			default:	break;
		}
		attroff(A_BLINK);
		refresh();
	} while( c!=KEY_F(1)  &&  c!=KEY_F(2)  &&  c!=KEY_F(3) );


	// Testa o conjunto de caracteres alternativos
	move(3,0);				// Move o cursor para inicio da linha 3
	printw("abcdefghijklmnopqrstuvxz059AM");
	move(4,0);				// Move o cursor para inicio da linha 4
	attron(A_ALTCHARSET);
	printw("abcdefghijklmnopqrstuvxz059AM");
	attroff(A_ALTCHARSET);
	refresh();


	// Movimentacao do cursor
	move(LINES-3,COLS-25);		// Move o cursor para antepultima linha, lado direito
	printw("Ultima Linha na direita");	// Escreve nas coordenadas atuais
	move(LINES-2,COLS-25);		// Move o cursor para penultima linha, lado direito
	printw("nLinhas=%d, nColunas=%d", LINES, COLS);
	refresh();					// Atualiza a tela com as partes modificadas


	init_pair(1, COLOR_RED, COLOR_BLACK);
	attron(COLOR_PAIR(1));			// Ativa par de cores '1'

	mvprintw(LINES-1,0,"Tecle qualquer coisa para terminar %.4f",123.456);
	refresh();
	getch();

	keypad(stdscr, FALSE);	// Retorna ao default
	noraw();		// Retorna ao default
	echo();			// Retorna ao default

	endwin();		// Encerra o modo curses



	// Terminal voltou ao normal
	printf("Voltou ao normal !!!\n");

	return 0;
}





