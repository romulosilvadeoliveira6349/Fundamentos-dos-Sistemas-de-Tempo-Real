#ifndef		CONSOLE_H
#define		CONSOLE_H



/** Código da thread para interface via console
*/
void console_threadConsole(void);


/** Inicializa o console
* 	Retorna -1 se erro
*/
int console_modoJanela(void);


/** Retorna o terminal ao modo normal
*/
void console_modoNormal(void);


/** Mostra na tela normal os tempos da simulação
*/
void console_showTempos(void);


/** Atualiza o terminal com os dados da planta, em modo janela
*/
void console_showPlantaTudo(void);




#endif		// CONSOLE_H







