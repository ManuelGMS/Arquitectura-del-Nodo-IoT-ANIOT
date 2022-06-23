/************** TAREAS********
 * Compila y ejecuta el código. Contesta a las cuestiones
 +
 + gcc array1.c -o main ; ./main
 +
 * 1. ¿Por qué no es necesario escribir "&list" para obtener la dirección del array list?
 +
 + Porque un array estático es realmente un puntero
 +
 * 2. ¿Qué hay almacenado en la dirección de "list"?
 +
 + Datos que ya estubieran antes en el STACK (cualquier cosa).
 +
 * 3. ¿Por qué es necesario pasar como argumento el tamaño del array en la función init_array?
 +
 + 
 + 
 * 4. ¿Por qué el tamaño (sizeof())  del array de la función "init_array" no coincide con el declarado en main()?
 +
 + Porque el sizeof() de una variable Pointer no devuelve el tamaño de lo que apunta si no del propio puntero.
 + Este tamaño depende de la arquitectura del procesaor: 8 bytes (sys 64 bits), 4 bytes (sys 32 bits), 2 bytes (sys 16 bits). 
 +
 * 5. ¿Por qué NO es necesario para omo argumento el tamaño del array en la función init_array2?
 +
 + Al valerse de la macro con la que se definió su tamaño no es necesario más parámetros en la función.
 +
 * 6. ¿Coincide el  tamaño (sizeof())  del array de la función "init_array2" con el declarado en main()?
 +
 + No, ocurre lo mismo que en el apartado 4.
 +
 *+**************/

#include <stdio.h>

#define N 5

void init_array(int array[], int size) ;

void init_array2(int array[N]);

int main(void) {
	
	int i, list[N];
	
	printf("Dir de list %p Dir de list[0]: %p  Dir de list[1]: %p. Sizeof list %lu\n\n",list,&list[0],&list[1],sizeof(list));
	
	init_array(list, N);
	
	for (i = 0; i < N; i++)
		printf("next: %d\n", list[i]);
	printf("\n-------------------------\n\n");

	init_array2(list);
	
	for (i = 0; i < N; i++)
		printf("next: %d\n", list[i]);
	printf("\n-------------------------\n\n");

}

void init_array(int array[], int size) {

	printf("Direccion de array: %p Sizeof array %lu \n", array, sizeof(array));
	
	for (int i = 0; i < size; i++) array[i] = i;
	
	printf("Array initialized\n\n");

}

void init_array2(int array[N]) {

	printf("Direccion de array: %p Sizeof array %lu \n", array, sizeof(array));
	
	for (int i = 0; i < N; i++) array[i] = i*2;

	printf("Array initialized\n\n");

}
