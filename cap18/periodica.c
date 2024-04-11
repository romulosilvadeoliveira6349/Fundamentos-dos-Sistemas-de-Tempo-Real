#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#define NSEC_POR_SEC	(1000000000)	// Numero de nanosegundos em um segundo (1 bilhao)
#define USEC_POR_SEC	(1000000)		// Numero de microssegundos em um segundo (1 milhao)
#define NSEC_POR_USEC	(1000)			// Numero de nanosegundos em um microsegundo (1 mil)



int main(int argc, char* argv[])
{
	struct timespec t;				// Hora atual
	struct timespec tp;				// Hora de inicio para o periodo de interesse

	int periodo_ns = 300000000;		// 300 ms = 300 000 000 ns
	long diferenca_us;				// Diferenca em microsegundos

	clock_gettime(CLOCK_MONOTONIC, &t);

	tp.tv_sec = t.tv_sec + 1;
	tp.tv_nsec = t.tv_nsec;

	for( int i=0; i<30; ++i) {

		// Espera ateh o inicio do proximo periodo
		clock_nanosleep( CLOCK_MONOTONIC, TIMER_ABSTIME, &tp, NULL);

		printf("------------------------ Acordei na iteracao %d\n", i);
		printf("Idealmente seria seg= %9ld   nseg= %09ld\n", tp.tv_sec, tp.tv_nsec);
		
		clock_gettime(CLOCK_MONOTONIC, &t);
		printf("Hora atual       seg= %9ld   nseg= %09ld\n", t.tv_sec, t.tv_nsec);
		
		diferenca_us = ( t.tv_sec - tp.tv_sec ) * USEC_POR_SEC + ( t.tv_nsec - tp.tv_nsec) / NSEC_POR_USEC;
		
		printf("Diferenca de %ld microsegundos\n", diferenca_us);

		// Calcula a proxima hora de acordar		
		tp.tv_nsec += periodo_ns;

		while (tp.tv_nsec >= NSEC_POR_SEC) {
			tp.tv_nsec -= NSEC_POR_SEC;
			tp.tv_sec++;
		}
	}
}





