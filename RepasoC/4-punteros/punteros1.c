/********* TAREAS ************
 * Compila y ejecuta el código. Responde a las siguientes cuestiones:
 +
 + gcc punteros1.c -o main ; ./main
 +
 * 1. ¿Qué operador utilizamos para declarar una variable como un puntero a otro tipo?
 +
 + El operador: *
 +
 * 2. ¿Qué operador utilizamos para obtener la dirección de una variable?
 +
 + El operador: &
 +
 * 3. ¿Qué operador se utiliza para acceder al contenido de la dirección "a la que apunta" un puntero?
 +
 + El operador: *
 +
 * 4. Hay un error en el código. ¿Se produce en  compilación o en ejecución? ¿Por qué se produce?
 +
 + El fallo se produce durante la ejecución del programa. Se produce por acceder a una posición de memoria restringida para el programa.
 +
 ***********/

#include <stdio.h>
#include <stdlib.h>

int c = 7;

int main(void) {

	int *ptr;
	printf("Direccion de ptr %p. ptr apunta a %p. Direccion de c: %p Valor de c %d\n", &ptr, ptr, &c, c);	

	ptr = &c;
	printf("Direccion de ptr %p,. ptr apunta a %p. Direccion de c: %p Valor de c %d\n",&ptr,ptr,&c,c);	
	
	*ptr=4;
	printf("ptr apunta a %p. Contenido de la direccion de ptr: %d Direccion de c: %p Valor de c %d\n",ptr,*ptr,&c,c);	

	// ptr =  (int*) 0x600a48;
	printf("Direccion de ptr %p. Valor de c %d\n",ptr,c);	
	
	*ptr = 13;
	printf("Direccion de ptr %p. Valor de c %d\n",ptr,c);	

	return 0;
	
}
