#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFSIZE	512

int main(int argc, char **argv) {

	int  n_read;
    FILE* ent, *sal;   	
    char buffer[BUFSIZE];
    
	// Comprobamos el número de argumentos.
	if(argc != 3) {
		perror("Los argumentos a pasar deben ser: <InputFileName> <OutputFileName>");
		exit(-1);
	}

	// Abre el archivo de entrada.
    ent = fopen(argv[1], "r"); 
	if (ent == NULL) {
        perror("fopen has failed");
	   	exit(-1);
    }
    
	// Crea el archivo de salida.
    sal = fopen(argv[2], "w+");
	if (sal == NULL) {
        fclose(ent);
        perror("create has failed");    
		exit(-1);
    }	   

    // Para leer del archivo de entrada
	// numberOfElementsReadCorrectly = fread(bufferWhereStoreInformation, sizeInBytesOfElementsToRead, numberOfElementsToRead, *FILE);
	while ((n_read = fread(buffer, sizeof(char), BUFSIZE, ent)) == BUFSIZE) 
		
		// numberOfElementsWritedCorrectly = fwrite(bufferWhereObtainInformation, sizeInBytesOfElementsToWrite, numberOfElementsToWrite, *FILE);
		if (fwrite(buffer, sizeof(char), n_read, sal) < n_read)  {
			perror("write has failed");
			fclose(ent); 
			fclose(sal);
			exit(-1);
		}

	// Hay que comprobar si se salió del bucle al alcanzar el EOF o si se salió por un error.
	if (feof(ent)) {
		
		// Se alcanzó el EOF, falta por realizar la última escritura.
		if (fwrite(buffer, sizeof(char), n_read, sal) < n_read)  {
			perror("write has failed");
			fclose(ent);
			fclose(sal);
			exit(-1);
		}

	} else { 
		perror("read has failed");
		fclose(ent);
		fclose(sal);
		exit(-1);
    } 

	fclose(ent); 
	fclose(sal);
	exit(0);

}
