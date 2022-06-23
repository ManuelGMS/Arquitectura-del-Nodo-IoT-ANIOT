/********* TAREAS ************
 * Compila y ejecuta el código. Responde a las siguientes cuestiones:
 +
 + gcc punteros2.c -o main ; ./main
 +
 * 1. ¿Cuánstos bytes se reservan en memoria con la llamada a malloc()?
 +
 + Se reservan: 4 bytes (int) * 127 (nelems) = 508 bytes
 +
 * 2. ¿Cuál es la dirección del primer y último byte de dicha zona reservada?
 +
 + Supongamos que la direccion del byte (0) es 0x564ce94fb2a0, por lo que 0x564ce94fb2a0 + (4 * 126 = 504 (_10) = 1f8 (_16)) 
 + es 0x557db2636498, esta sera la direccion del primer byte del ultimo elemento accesible. Como un int ocupa 4 bytes, solo 
 + hay que sumar 3 bytes a la dirección 0x563920d72498, por lo que la ultima direccion de la zona reservada es: 0x557db263649c
 +
 * 3. ¿Por qué el contenido de la dirección apuntada por "ptr" es 7 y no 5 en el primer printf()?
 +
 + Las lineas "*ptr = 5;" y "ptr[0] = 7;" apuntan al principio del array "ptr", como la ultima instruccion 
 + es la asignacion del valor 7, ese es el valor que devuelve el contenido de aquello a lo que apunta "ptr".
 +
 * 4. ¿Por qué se modfica el contenido de ptr[1] tras la sentencia *ptr2=15;?
 +
 + Al principio ptr y ptr2 apuntan al mismo sitio, luego ptr2++ hace que la posicion del puntero se desplace
 + a la del siguiente elemento del array, es por eso que se modifica el contenido de ptr[1].
 +
 * 5. Indica dos modos diferentes de escribir el valor 13 en la dirección correspondiente a ptr[100].
 +
 + Modo 1: *(ptr+100)=13;
 + Modo 2: ptr+=100; *ptr=13;
 +
 * 6. Hay un error en el código. ¿Se manifiesta en compilación o en ejecución? Aunque no se manifieste, el error está. ¿Cuál es?
 +
 + Se da un error en tiempo de ejecución, esto se debe a que primero se libera la memoria dinamica a la que ptr apunta y luego
 + se intenta volver a escribir en ella.
 +
 *******************/

#include <stdio.h>
#include <stdlib.h>

int nelem;

int main(void) {
	
	int *ptr;
	int * ptr2;	

	nelem = 127;
	ptr = (int*) malloc(nelem*sizeof(int));
	*ptr = 5;
	ptr[0] = 7;
	ptr2 = ptr;

	printf("%ld ; %lu ; %p ; %p\n", nelem * sizeof(int), sizeof(int), ptr, ptr+126);

	for (size_t i = 0; i < nelem; i++) printf("%d ; %p\n", i, ptr+i);
	
	printf("Direccion a la que apunta ptr %p. Contenido de esa direccion: %d \n",ptr,*ptr);

	ptr[1] = 10;	
	printf("Direccion a la que apunta ptr[1] %p. Contenido de esa direccion: %d \n",&ptr[1],ptr[1]);	

	ptr2++;
	*ptr2 = 15;
	printf("Direccion a la que apunta ptr[1] %p. Contenido de esa direccion: %d \n",&ptr[1],ptr[1]);	

	// free(ptr);	
	*ptr = 3;
	printf("Direccion a la que apunta ptr %p. Contenido de esa direccion: %d \n",ptr,*ptr);	

	free(ptr);
	
}
