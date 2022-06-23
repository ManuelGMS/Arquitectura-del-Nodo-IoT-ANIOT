/**** TAREAS ********
 * Compila el codigo, ejecutalo y responde a las cuestiones
 + 
 + gcc main1.c -o main ; ./main
 +
 * 1. ¿Por qué el primer printf() imprime valores distintos para 'a'?
 + 
 + (%d) indica que el valor de salida wa un número entero; (%c) indica que el valor es el caracter correspondiente al código ascii pasado como parámetro. 
 +
 * 2. ¿Cuánto ocupa un tipo de datos char?
 + 
 + 1 byte, o lo que es lo mismo, 8 bits. 
 +
 * 3. ¿Por qué el valor de 'a' cambio tanto al incrementarlo en una unidad? (la  respuesta está relacionada con la cuestión anterior)
 + 
 + A menos que se aplique un modificador al tipo char, la varible vista como un entero va de -128 a 127. Si todo el rango fuese entero iría de 0 a 255.
 +
 * 4. Si un "long" y un "double" ocupan lo mismo, ¿por qué hay 2 tipos de datos diferentes?
 + 
 + Los tipos de datos basicos de C son: signed, unsigned, short y long.
 + El tipo char permite definir variables que sirven para trabajar con caracteres.
 + El tipo int permite definir variables que sirven para trabajar con números enteros.
 + Los tipos double y float permiten definir variables que sirven para trabajar con números decimales. Double implica una mayor precisión decimal.
 +
 + Las siguientes palabras son conocidas como modificadores: signed, unsigned, short y long
 + Los modificadores se anteponen a los tipos básicos para aumentar o disminuir la cantidad de memoria asignada a una variable.
 +
 + Es posible utilizar long como un tipo de variable de tamaño 8 bytes que trabaja con enteros, y no con decimales.
 +
*********************/

#include <stdio.h>

int b = 41;
char a = 127;

int main() {

	printf("a = %d a = %c\n", a, a);
	a++;
	printf("a = %d a = %c b=%d  b=%c\n", a,a,b,b);
	printf("Size of int: %lu\n",sizeof(int ) );
	printf("Size of char: %lu\n",sizeof( char) );
	printf("Size of float: %lu\n",sizeof(float ) );
	printf("Size of double: %lu\n",sizeof( double) );
	printf("Size of long: %lu\n",sizeof(long ) );
	printf("Size of short: %lu\n",sizeof( short) );
	printf("Size of void*: %lu\n",sizeof( void*) );

}
