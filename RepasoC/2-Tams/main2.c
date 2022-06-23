/******** TAREAS *****
 * Compila el código y trata de ejecutarlo. Contesta a las cuestiones
 +
 + gcc main2.c -o main ; ./main
 +
 * 1. ¿Tenemos un problema de compilación o de ejecución?
 +
 + gcc main2.c -o main --> Falla --> Fallo de compilación.
 +
 * 2. ¿Por qué se da el problema? Soluciónalo, compila y ejecuta de nuevo.
 +
 + int array2[a];
 + El problema reside en que se intenta dar el tamaño de un array en base a una variable que no es constante, si no de rango varible.
 + Para esta tarea es posible utilizar la directiva #define para crear una constante que es reemplazada en la fase de preprocesamiento.
 +
 * 3. a,b,c, y x están declaradas de forma consecutiva. ¿Sus direcciones son concesutivas?
 +
 + Las variables no se almacenan de forma consecutiva en el STACK.
 +
 * 4. ¿Qué significa el modificar "%lu" en printf()?
 +
 + Significa Unsigned Long (Integer), permitiría mostrar enteros mas grandes de un rango de valores únicamente positivo.
 +
 * 5. ¿A qué dirección apunta "pc"? ¿Coincide con la de alguna variable anteriormente declarada? Si es así ¿Coinciden los tamaños de ambas?
 +
 + La dirección devuelta por una variable de tipo puntero es la de aquella variable a la que apunta, X en este caso.
 + Para ver cual es la dirección de memoria del puntero en sí: &pointer.
 +
 * 6. ¿Coincide el valor del tamaño de "array1" con el número de elementos del array? ¿Por qué?
 +
 + No coinciden. Array tiene 10 elementos, pero su tamaño en memoria es de: 4 bytes (int) * 10 (nº elementos) = 40 bytes.
 +
 * 7. ¿Coinciden las direcciones a la que apunta cadena1 con la de cadena2?
 +
 + No, el comienzo de cara array en la memoria es diferente.
 +
 * 8. ¿Por qué los tamaños (según sizeof()) de cadena1 y cadena2 son diferentes?
 +
 + char* cadena1 = "CADENA DE CARACTERES";	// Puntero a chars: el tamaño de un puntero a chars varía con la arquitectura del procesador 8 bytes (64-bit), 4 bytes (32-bit) 2 bytes (16-bit). 
 + char cadena2[] = "CADENA DE CARACTERES"; // Array, su tamaño es: sizeof(type) * numElements. Para los array de caracteres operador sizeof cuenta el byte \0 como propio de la string (+1).
 +
**********************/

#include <stdio.h>

#define ARRAY_SIZE  10

int a = 7;
unsigned long b = 8;
short c;
char x;

char* pc;

int array1[ARRAY_SIZE];
int array2[ARRAY_SIZE]; 

// Cuando un array de char se declara como Var = "something", el compilador mete por defecto un \0 al finalizar la secuencia, pero no en el resto de las inicializaciones.
char* cadena1 = "CADENA DE CARACTERES";	// Puntero a chars: el tamaño de un puntero a chars varía con la arquitectura 8 bytes (64-bit), 4 (32-bit) 2 (16-bit). 
char cadena2[] = "CADENA DE CARACTERES"; // Array, su tamaño es: sizeof(type) * numElements. Para los array de caracteres operador sizeof cuenta el byte \0 como propio de la string (+1).

int main() {
	
	pc =&x;
	a = 16;
	printf("+%s+ ; %ld \n", cadena1, sizeof(cadena1));
	printf("+%s+ ; %ld \n", cadena2, sizeof(cadena2));
	printf("Direccion de a: %p Tam: %lu \n",&a,sizeof(a));
	printf("Direccion de b: %p Tam: %lu \n",&b,sizeof(b));
	printf("Direccion de c: %p Tam: %lu \n",&c,sizeof(c));
	printf("Direccion de x: %p Tam: %lu \n",&x,sizeof(x));
	printf("Direccion de pc: %p Direccion a la que apunta pc: %p Tam: %lu \n",&pc,pc,sizeof(pc));
	printf("Direccion de array: %p Direccion del elem 0: %p Tam de array: %lu \n",array1, &array1[0], sizeof(array1));
	printf("Direccion de cadena1: %p Direccion a la que apunta: %p Tam: %lu \n",&cadena1,cadena1,sizeof(cadena1));
	printf("Direccion de cadena2: %p Direccion a la que apunta: %p Tam: %lu \n",&cadena2,cadena2,sizeof(cadena2));	
	return 0;

}
