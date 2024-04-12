#ifndef		INSTRUMENTACAO_H
#define		INSTRUMENTACAO_H



/**	Thread servidora que recebe comandos para a instrumentacao
*	Formato do protocolo é string com campos separados por espaços
*	Responde ? quando não entendeu o argumento, exemplo "bombacoletor ?"
*
*	Pedido para ligar/desligar a bomba do coletor
*		"bombacoletor"
*		<novo estado: 1 ou 0>
*	Resposta 
*		"zbombacoletor"
*		<novo estado: 1 ou 0>
*
*
*	Pedido para ligar/desligar a bomba de circulacao
*		"bombacirculacao"
*		<novo estado: 1 ou 0>
*	Resposta 
*		"zbombacirculacao"
*		<novo estado: 1 ou 0>
*
*
*	Pedido para ligar/desligar o aquecedor eletrico
*		"aquecedor"
*		<novo estado: 1 ou 0>
*	Resposta 
*		"zaquecedor"
*		<novo estado: 1 ou 0>
*
*
*	Pedido para ligar/desligar a válvula de entrada
*		"valvulaentrada"
*		<novo estado: 1 ou 0>
*	Resposta 
*		"zvalvulaentrada"
*		<novo estado: 1 ou 0>
*
*
*	Pedido para ligar/desligar a válvula de esgoto
*		"valvulaesgoto"
*		<novo estado: 1 ou 0>
*	Resposta 
*		"zvalvulaesgoto"
*		<novo estado: 1 ou 0>
*
*
*	Pedido para ler o nível do boiler
*		"nivelboiler"
*	Resposta 
*		"znivelboiler"
*		<nivel:float>			nível em metros
*
*
*	Pedido para ler a temperatura do boiler
*		"tempboiler"
*	Resposta
*		"ztempboiler"
*		<temperatura:float>		temperatura em graus celsius
*
*
*	Pedido para ler a temperatura do coletor
*		"tempcoletor"
*	Resposta
*		"ztempcoletor"
*		<temperatura:float>		temperatura em graus celsius
*
*
*	Pedido para ler a temperatura dos canos
*		"tempcanos"
*	Resposta
*		"ztempcanos"
*		<temperatura:float>		temperatura em graus celsius
*
*/
void instrumentacao_threadServidora(void);



/** Envia string para o controlador sem esperar resposta
*/
void instrumentacao_enviaControlador(char *msg);



/**	Inicializa o módulo de instrumentação
*	porta - número da porta UDP para escutar
*/
void instrumentacao_inicializa( int porta);



#endif		// INSTRUMENTACAO_H


