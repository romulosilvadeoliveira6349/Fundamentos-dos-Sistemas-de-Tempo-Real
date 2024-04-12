#ifndef		PLANTA_H
#define		PLANTA_H


// Número de dados para uma descrição completa da planta
#define	PLANTA_DADOS		24	



/** Le estado da bomba do coletor
*/
int planta_leBombaColetor(void);


/** Le estado da bomba de circulação
*/
int planta_leBombaCirculacao(void);


/** Le estado do aquecedor elétrico
*/
int planta_leAquecedor(void);


/** Le estado da válvula de entrada
*/
int planta_leValvulaEntrada(void);


/** Le estado da válvula de esgoto
*/
int planta_leValvulaEsgoto(void);


/** Le vazao de consumo
*/
double planta_leVazaoConsumo(void);


/** Define estado da bomba do coletor
*/
void planta_acionaBombaColetor( int liga);


/** Define estado da bomba de circulação
*/
void planta_acionaBombaCirculacao( int liga);


/** Define estado do aquecedor elétrico
*/
void planta_acionaAquecedor( int liga);


/** Define estado da válvula de entrada
*/
void planta_acionaValvulaEntrada( int liga);


/** Define estado da válvula de esgoto
*/
void planta_acionaValvulaEsgoto( int liga);


/** Define vazao de consumo
*/
void planta_defineVazaoConsumo( double vazao);


/** Retorna nível do boiler [m]
*/
double planta_leNivelBoiler(void);


/** Retorna temperatura no boiler [Celsius]
*/
double planta_leTempBoiler(void);


/** Retorna temperatura da água no coletor solar [Celsius]
*/
double planta_leTempColetor(void);


/** Retorna temperatura da água nos canos [Celsius]
*/
double planta_leTempCanos(void);


/** Mostra na tela todas as variáveis de estado
*/
void planta_mostraTudo( void);


/** Cria a planta
*/
void planta_cria(void);


/** Executa um passo da simulação
*/
void planta_tickUpdate(int passo);


/** Indica se um erro já foi detectado
*/
int planta_leErroDetectado(void);


/* Texto para os dados completos
*/
char **planta_textosCompletos(void);


/** Retorna todos os dados disponíveis sobre a planta
*/
void planta_dadosCompletos(double *dados);




#endif		// PLANTA_H




