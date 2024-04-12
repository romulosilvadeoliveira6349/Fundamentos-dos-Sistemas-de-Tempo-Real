#include	<stdio.h>
#include	<stdlib.h>
#include	<assert.h>
#include	<pthread.h>

#include	"planta.h"

#include	"solar2022.h"


// Mutex para compartilhar os dados entre várias threads
static pthread_mutex_t em_dados = PTHREAD_MUTEX_INITIALIZER;



/*	Descrição da planta */

/* Definições */

static double calorEspecifico = 4184.0;		// Calor especidico da água [ Joule/Kg.Celsius]

static double tempMaxima = 80.0;			// Temperatura máxima em qualquer lugar
static double tempMinima = 1.0;				// Temperatura mínima em qualquer lugar

static double tempEntrada = 16.0;			// Temperatura da água da rede (entrada) [Celsius]
static double vazaoEntrada = 35.0/60.0;		// Vazão de entrada [l/s]
static double vazaoEsgotamento = 45.0/60.0;	// Vazão de esgotamento [l/s]
static double vazaoBomba = 50.0/60.0;		// Vazão de uma bomba [l/s]
static double potenciaBoiler = 8000.0;		// Potência do aquecedor elétrico no boiler [J/s]

static double capacidadeBoiler = 1200.0;	// Capacidade do boiler [l]
static double areaBase = 2.0;				// Área da base do boiler [m^2]  
static double volumeColetor = 1800.0;		// Capacidade do coletor [l]
static double volumeCanos = 45.0;			// Capacidade dos canos [l]

             
/* Acionamento on/off */
static int bombaColetor = 0;		// Indica bomba do coletor ligada ou não
static int bombaCirculacao = 0;		// Indica bomba de recirculação ligada ou não
static int aquecedorEletrico = 0;	// Indica aquecedor ligado ou não
static int valvulaEntrada = 0;		// Indica entrada de água aberta ou não
static int valvulaEsgoto = 0;		// Indica saída de água para esgoto aberta ou não


/* Dinâmica da planta */
static double volumeBoiler = 1000;			// Volume de água no boiler [l]
static double nivelBoiler = 1000/2000.0;	// Nível de água no boiler [m]  	
static double tempBoiler = 25.0;			// Temperatura da água no boiler [Celsius]
static double tempColetor = 40.0;			// Temperatura da água no coletor solar [Celsius]
static double tempCanos = 25.0;				// Temperatura da água nos canos [Celsius]


/* Perturbação */
static double vazaoConsumo = 0.0;			// Vazão de consumo atual [l/s]
static double vazaoConsumoMaxima = 2.0;		// Vazão de consumo máxima [l/s]


/* Condições de erro pré-definidas */
static int erroDetectado;					// Indica se um erro foi detectada


/* Texto para os dados completos
*/
static char *texto_dados[PLANTA_DADOS] = {
	"Temperatura maxima no sistema (Celsius)       ",
	"Temperatura minima no sistema (Celsius)       ",
	"Temperatura de entrada (Celsius)              ",
	"Vazao de entrada (l/s)                        ",
	"Vazao de esgotamento (l/s)                    ",
	"Vazao das bombas (l/s)                        ",
	"Potencia do aquecedor eletrico no boiler (J/s)",
	"Capacidade do boiler (l)                      ",
	"Area da base do boiler (m^2)                  ",
	"Volume de agua no coletor (l)                 ",
	"Volume de agua nos canos (l)                  ",
	"Volume de agua no boiler (l)                  ",
	"Nivel de agua no boiler (m)                   ",
	"Temperatura da agua no boiler (Celsius)       ",
	"Temperatura da agua no coletor solar (Celsius)",
	"Temperatura da agua nos canos (Celsius)       ",
	"Vazao de consumo maxima (l/s)                 ",
	"Vazao de consumo (l/s)                        ",
	"Indica bomba do coletor ligada ou nao         ",
	"Indica bomba de recirculação ligada ou nao    ",
	"Indica aquecedor ligado ou nao                ",
	"Indica entrada de agua aberta ou nao          ",
	"Indica saida de agua para esgoto aberta ou nao",
	"Erro foi detectado ou nao                     "
	};





/** Le estado da bomba do coletor
*/
int planta_leBombaColetor(void) {
	return bombaColetor;
}

/** Le estado da bomba de circulação
*/
int planta_leBombaCirculacao(void) {
	return bombaCirculacao;
}

/** Le estado do aquecedor elétrico
*/
int planta_leAquecedor(void) {
	return aquecedorEletrico;
}

/** Le estado da válvula de entrada
*/
int planta_leValvulaEntrada(void) {
	return valvulaEntrada;
}

/** Le estado da válvula de esgoto
*/
int planta_leValvulaEsgoto(void) {
	return valvulaEsgoto;
}	

/** Le vazao de consumo
*/
double planta_leVazaoConsumo(void) {
	return vazaoConsumo;
}	



/** Define estado da bomba do coletor
*/
void planta_acionaBombaColetor( int liga)
{
	assert( liga==0 || liga==1);
	bombaColetor = liga;
}


/** Define estado da bomba de circulação
*/
void planta_acionaBombaCirculacao( int liga)
{
	assert( liga==0 || liga==1);
	bombaCirculacao = liga;
}


/** Define estado do aquecedor elétrico
*/
void planta_acionaAquecedor( int liga)
{
	assert( liga==0 || liga==1);
	aquecedorEletrico = liga;
}



/** Define estado da válvula de entrada
*/
void planta_acionaValvulaEntrada( int liga)
{
	assert( liga==0 || liga==1);
	valvulaEntrada = liga;
}


/** Define estado da válvula de esgoto
*/
void planta_acionaValvulaEsgoto( int liga)
{
	assert( liga==0 || liga==1);
	valvulaEsgoto = liga;
}


/** Define vazao de consumo
*/
void planta_defineVazaoConsumo( double vazao)
{
	if( vazao < 0 )
		vazao = 0;

	if( vazao > vazaoConsumoMaxima )
		vazao = vazaoConsumoMaxima;

	vazaoConsumo = vazao;
}


/** Retorna nível do boiler [m]
*/
double planta_leNivelBoiler(void)
{
	return nivelBoiler;
}


/** Retorna temperatura no boiler [Celsius]
*/
double planta_leTempBoiler(void)
{
	double aux;
	pthread_mutex_lock(&em_dados);
	aux = tempBoiler;
	pthread_mutex_unlock(&em_dados);
	return aux;
}


/** Retorna temperatura da água no coletor solar [Celsius]
*/
double planta_leTempColetor(void)
{
	return tempColetor;
}


/** Retorna temperatura da água nos canos [Celsius]
*/
double planta_leTempCanos(void)
{
	return tempCanos;
}


/** Mostra na tela todas as variáveis de estado
*/
void planta_mostraTudo(void)
{
	printf("\033[H\033[J");		// Limpa a tela	
	
	pthread_mutex_lock(&em_dados);
	printf("Temperatura máxima no sistema (Celsius) %0.3lf\n", tempMaxima);
	printf("Temperatura mínima no sistema (Celsius) %0.3lf\n", tempMinima);
	printf("Temperatura da água de entrada (Celsius) %0.3lf\n", tempEntrada);
	printf("Vazão de entrada (l/s) %0.3lf\n", vazaoEntrada);
	printf("Vazão de esgotamento (l/s) %0.3lf\n", vazaoEsgotamento);
	printf("Vazão das bombas (l/s) %0.3lf\n", vazaoBomba);
	printf("Potência do aquecedor elétrico no boiler (J/s) %0.3lf\n", potenciaBoiler);
	printf("Capacidade do boiler (l) %0.3lf\n", capacidadeBoiler); 
	printf("Área da base do boiler (m^2) %0.3lf\n", areaBase);
	printf("Volume de água no coletor (l) %0.3lf\n", volumeColetor); 
	printf("Volume de água nos canos (l) %0.3lf\n", volumeCanos); 
	printf("Volume de água no boiler (l) %0.3lf\n", volumeBoiler); 
	printf("Nível de água no boiler (m) %0.3lf\n", nivelBoiler);
	printf("Temperatura da água no boiler (Celsius) %0.3lf\n", tempBoiler);
	printf("Temperatura da água no coletor solar (Celsius) %0.3lf\n", tempColetor);
	printf("Temperatura da água nos canos (Celsius) %0.3lf\n", tempCanos);
	printf("Vazão de consumo máxima (l/s) %0.3lf\n", vazaoConsumoMaxima);
	printf("Vazão de consumo (l/s) %0.3lf\n", vazaoConsumo);
	printf("Indica bomba do coletor ligada ou não %d\n", bombaColetor);
	printf("Indica bomba de recirculação ligada ou não %d\n", bombaCirculacao);
	printf("Indica aquecedor ligado ou não %d\n", aquecedorEletrico);
	printf("Indica entrada de água aberta ou não %d\n", valvulaEntrada);
	printf("Indica saída de água para esgoto aberta ou não %d\n", valvulaEsgoto);
	printf("Erro foi detectado ou não %d\n", erroDetectado);	
	pthread_mutex_unlock(&em_dados);
}



/* Texto para os dados completos
*/
char **planta_textosCompletos(void)
{
	return texto_dados;
}


/** Retorna todos os dados disponíveis sobre a planta
*/
void planta_dadosCompletos(double *dados)
{
	pthread_mutex_lock(&em_dados);
	dados[0] = tempMaxima;
	dados[1] = tempMinima;
	dados[2] = tempEntrada;
	dados[3] = vazaoEntrada;
	dados[4] = vazaoEsgotamento;
	dados[5] = vazaoBomba;
	dados[6] = potenciaBoiler;
	dados[7] = capacidadeBoiler; 
	dados[8] = areaBase;
	dados[9] = volumeColetor; 
	dados[10] = volumeCanos; 
	dados[11] = volumeBoiler; 
	dados[12] = nivelBoiler;
	dados[13] = tempBoiler;
	dados[14] = tempColetor;
	dados[15] = tempCanos;
	dados[16] = vazaoConsumoMaxima;
	dados[17] = vazaoConsumo;
	dados[18] = bombaColetor;
	dados[19] = bombaCirculacao;
	dados[20] = aquecedorEletrico;
	dados[21] = valvulaEntrada;
	dados[22] = valvulaEsgoto;
	dados[23] = erroDetectado;	
	assert( 24 == PLANTA_DADOS );
	pthread_mutex_unlock(&em_dados);
}


/** Cria a planta
*/
void planta_cria(void)
{
	erroDetectado = 0;
}



/** Executa um passo da simulação
*
*	deltaEnergia = calorEspecifico * volume * deltaTemperatura; 
*	deltaTemperatura = deltaEnergia / ( calorEspecifico * volume );
*/
void planta_tickUpdate(int passo)
{	
	double novoVolumeBoiler;
	double novoNivelBoiler;
	double novoTempBoiler;
	double novoTempColetor;
	double novoTempCanos;

	double deltaEnergia;


	pthread_mutex_lock(&em_dados);


	/* Volume de água no boiler [l] */
	novoVolumeBoiler = volumeBoiler;

	if( valvulaEntrada == 1 )
		novoVolumeBoiler += (passo/1000.0) * vazaoEntrada;
		
	if( valvulaEsgoto == 1 )
		novoVolumeBoiler -= (passo/1000.0) * vazaoEsgotamento;

	novoVolumeBoiler -= (passo/1000.0) * vazaoConsumo;


	/* Nível de água no boiler [m] */
	novoNivelBoiler = (novoVolumeBoiler / 1000) / areaBase;


	/* Temperatura da água nos canos [Celsius] */
	novoTempCanos = tempCanos;
	deltaEnergia = 0.0;

	deltaEnergia -= calorEspecifico * volumeCanos * (0.5/60) * (passo/1000.0); // Cai 0.5 graus por minuto

	if( vazaoConsumo > 0 ) {
		double movido = (passo/1000.0) * vazaoConsumo;
		deltaEnergia += calorEspecifico * movido * (tempBoiler-tempCanos); 
	}

	if( bombaCirculacao == 1 ) {
		double movido = (passo/1000.0) * vazaoBomba;
		deltaEnergia += calorEspecifico * movido * (tempBoiler-tempCanos);
	}

	novoTempCanos += deltaEnergia / ( calorEspecifico * volumeCanos);


	/* Temperatura da água no coletor solar [Celsius] */
	novoTempColetor = tempColetor;
	deltaEnergia = 0.0;

	deltaEnergia += calorEspecifico * volumeColetor * (1.0/60) * (passo/1000.0); // Sobe 1.0 grau por minuto

	if( bombaColetor == 1 ) {
		double movido = (passo/1000.0) * vazaoBomba;
		deltaEnergia += calorEspecifico * movido * (tempBoiler-tempColetor); 
	}

	novoTempColetor += deltaEnergia / ( calorEspecifico * volumeColetor);


	/* Temperatura da água no boiler [Celsius] */
	novoTempBoiler = tempBoiler;
	deltaEnergia = 0.0;

	deltaEnergia -= calorEspecifico * volumeBoiler * (0.1/60) * (passo/1000.0); // Cai 0,1 graus por minuto
	//printf("\n\n#1## %lf\n\n", deltaEnergia);

	if( aquecedorEletrico == 1 )
		deltaEnergia += potenciaBoiler * (passo/1000.0);
	//printf("\n\n#2## %lf\n\n", deltaEnergia);

	if( valvulaEntrada == 1 ) {
		double movido = (passo/1000.0) * vazaoEntrada;
		deltaEnergia += calorEspecifico * movido * (tempEntrada-tempBoiler); 
	}
	//printf("\n\n#3## %lf\n\n", deltaEnergia);

	if( bombaColetor == 1 ) {
		double movido = (passo/1000.0) * vazaoBomba;
		deltaEnergia += calorEspecifico * movido * (tempColetor-tempBoiler); 
	}
	//printf("\n\n#4## %lf\n\n", deltaEnergia);

	if( bombaCirculacao == 1 ) {
		double movido = (passo/1000.0) * vazaoBomba;
		deltaEnergia += calorEspecifico * movido * (tempCanos-tempBoiler); 
	}
	//printf("\n\n#5## %lf\n\n", deltaEnergia);

	novoTempBoiler += deltaEnergia / ( calorEspecifico * volumeBoiler);
	//printf("\n\n#6## %lf\n\n", novoTempBoiler);


	/* Assume os novos valores */
	volumeBoiler = novoVolumeBoiler;
	nivelBoiler = novoNivelBoiler;
	tempBoiler = novoTempBoiler;
	tempColetor = novoTempColetor;
	tempCanos = novoTempCanos;
	
	/* Limites ambientais arbitrários */
	tempCanos = tempCanos > tempMaxima ? tempMaxima : tempCanos;
	tempCanos = tempCanos < tempMinima ? tempMinima : tempCanos;
	tempColetor = tempColetor > tempMaxima ? tempMaxima : tempColetor;
	tempColetor = tempColetor < tempMinima ? tempMinima : tempColetor;
	tempBoiler = tempBoiler > tempMaxima ? tempMaxima : tempBoiler;
	tempBoiler = tempBoiler < tempMinima ? tempMinima : tempBoiler;

	/* Detecta erro: volume no boiler muito pequeno */
	if( volumeBoiler < 0.1 ) {
		erroDetectado = 1;
		volumeBoiler = 0;
		nivelBoiler = 0;
	}
	
	/* Detecta erro: volume no boiler muito grande */
	if( volumeBoiler > capacidadeBoiler ) {
		erroDetectado = 1;
		volumeBoiler = capacidadeBoiler;
		nivelBoiler = (capacidadeBoiler / 1000) / areaBase;
	}


	pthread_mutex_unlock(&em_dados);
}



/** Indica se um erro já foi detectado
*/
int planta_leErroDetectado(void)
{
	return erroDetectado;
}









