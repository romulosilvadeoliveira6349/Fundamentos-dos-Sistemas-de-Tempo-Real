#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>



void main(void) {

	char linha[100];

	printf("Digite uma frase:\n");

	fgets( linha, 100, stdin);

	printf("A frase tem %ld caracteres.\n", strlen(linha)-1 );
}



