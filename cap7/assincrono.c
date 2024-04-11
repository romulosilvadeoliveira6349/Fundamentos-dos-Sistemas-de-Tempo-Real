#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>


void main(void) {

	char linha[100];
	char *x;
	int flags = fcntl( fileno(stdin), F_GETFL, 0);
	fcntl( fileno(stdin), F_SETFL, flags | O_NONBLOCK);

	printf("Digite uma frase:\n");
	x = fgets( linha, 100, stdin);

	while( x == NULL ) {
		printf(".");
		x = fgets( linha, 100, stdin);
	}

	printf("A frase tem %ld caracteres.\n", strlen(linha)-1 );
}



