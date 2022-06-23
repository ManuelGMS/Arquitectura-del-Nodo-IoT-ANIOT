#include <stdio.h>
#include <string.h>

/********* TAREAS ************
 * Compila y ejecuta el código. Responde a las siguientes cuestiones:
 +
 + gcc strings2.c -o main ; ./main
 +
 * 1. El código de "copia" no funciona. ¿Por qué? 
 +
 + Porque solo se está cambiando la asignación de los punteros locales.
 +
 * 2. Usa ahora la función copia2() (descomenta la línea correspondiente). ¿Funciona la copia?
 +
 + Claro, en copia2 si porque estamos manipulando directamente la dirección a la que ha de apuntar el puntero cad2 del main.
 +
 * 3. Propón una implementación correcta de la copia.
 +
 + Modo 1: Con un bucle FOR.
 + Modo 2: Usar la función memcpy.
 +
 * 4. ¿Qué hace la funcióbn "mod"? ¿Por qué funciona?
 +
 + La función copia ORIGEN en DESTINO pero todo en mayúsculas, funciona porque se manipulan los caracteras a través de su código ascii.
 +
 * 5. Descomenta la última llamada a la función "mod". Compila y ejecuta. ¿Por qué se produce el error?
 +
 + 
 +
 *****************************/

void copia2(char* org, char** dst) {
	*dst = org;
}

void copia(char* org, char* dst) {
	dst = org;
}

void mod(char* org, char* dst) {

	// for (int i=0;i<strlen(org);i++)
		// dst[i] = org[i] - 32;

	// Se añade esta línea.
	// dst[strlen(org) + 1] = '\0';

}

int main() {

	char* cad1 = "original";          
	char* cad2 = "otra";
	char cad3[32];

	copia(cad1, cad2);
	copia2(cad1, &cad2);
	printf("cad1 %s cad2 %s\n", cad1,cad2);

	mod(cad1,cad3);
	printf("cad1 %s cad3 %s\n", cad1,cad3);
	
	mod(cad1,cad1);

}

