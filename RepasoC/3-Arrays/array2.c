/************ TAREAS
 * Compila y ejecuta el código. Contesta a las cuestiones:
 +
 + gcc array2.c -o main ; ./main
 +
 * 1. ¿La copia del array se realiza correctamente? ¿Por qué?
 +
 + No, porque al pasar un array lo único que se copia es el puntero no el contenido.
 + A parte que lo único que cambia si acaso es la dirección a la que apuntan los
 + punteros (src, dst) que son locales a la función, y no A y B.
 +
 * 2. Si no es correcto, escribe un código que sí realice la copia correctamente.
 +
 + Se podría hacer con un bucle FOR, pero se opta por usar la función memcpy de string.h
 +
 * 3. Descomenta la llamada a la fución "tmo" en la función main(). Compila de nuevo y ejecuta.
 +
 + Se produce un error.
 +
 * 4. El problema que se produce, ¿es de compilación o de ejecución? ¿Por qué se produce?
 +
 + Es un problema que tiene lugar en ejecución.
 + Se produce porque se intenta leer fuera de las posiciones de un array.
 +
 * 5. Encuentra un valor de MAXVALID superior a 4 con el que no se dé el problema.
 * ¿Se está escribiendo más allá del tamaño del array? Si es así, ¿por qué funciona el código?
 +
 + 
 +
*******************/

#include <stdio.h>
#include <string.h>

#define N 10
#define MAXELEM 5000
#define MAXVALID 100

// funcion que imprime por pantalla el conteniod del array v de tam. size
void imprimeArray(int v[],int size) {
	printf("-------------------\n\n");
	for (int i=0;i<size;i++)
		printf("%d+ ", v[i]);
	printf("\n\n");

}

// Funcion que copia el contenido de un array en otro.
void copyArray(int src[],int dst[], int size) {
	/*
	En C los arrays se pasan como un puntero al primer
	elemento, no existe realmente la posibilidad de
	realizar un paso por valor, ya que lo único que se
	copia es el valor del puntero y no los elemntos que
	son apuntados por este.

	En este caso src -> A y dst -> B y luego dst -> A,
	pero cuando acabe la función A y B seguirán como antes.
	*/
	// dst = src;

	// memcpy (string.h) copia la cantidad de bytes de src en dst que se indiquen en el tercer campo.
 	memcpy(dst, src, N * sizeof(int));
}

void tmo() {

	int x = -1; // Posicion de memoria más baja.
	int a[4] = {0,1,2,3}; // Posición de memoria más alta.	 
	int b = 10000;
	int c = -1;

	printf("x: %p ; b: %p ; c: %p\n", &x, &b, &c);
	printf("a[0]: %p ; a[1]: %p ; a[2]: %p ; a[3]: %p\n",a+0, a+1, a+2, a+3);
	
	for (int i=4;i<6;i++) { // MAXVALID
		a[i] = i;
		printf("Iteration: %d ; a[%d]: %p\n", i, i, &a[i]);
	} 
	printf("x %d b %d c %d\n", x,b,c); 
			
}

int main() {

	int A[N] = {4,3,8,5,6,9,0,1,7,2};
	int B[N];

	printf("Dir A[0]: %p\nDir A[9]: %p\n", &A[0], &A[9]);
	printf("Dir B[0]: %p\nDir B[9]: %p\n", &B[0], &B[9]);

	tmo();
	copyArray(A,B,N);
	imprimeArray(B,N);
		
}
