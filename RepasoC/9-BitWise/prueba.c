#include <stdio.h>
/********* TAREAS ************
 * Compila y ejecuta el código.
 +
 + gcc prueba.c -o main ; ./main
 +
 * 1. ¿Por qué la escritura usando el puntero 'p' no sobreescribe todo el valor de 'a'?
 +
 + Porque el puntero a char solo puede maniular un máximo de 1 byte / 8 bits.
 +
 * 2. ¿Cuánto se modifica la dirección de 'p' tras la sentencia 'p=p+1'?
 +
 + La direccion de "p" sería la dirección de "a" + 1 (byte).
 +
 * 3. ¿Qué habría variado si 'p' fuese del tipo 'short *' ?
 +
 + El tipo short son 2 bytes, por lo que no habría hecho falta la aritmética de punteros para
 + modificar el primer y el segundo byte al mismo tiempo.
 +
 ************** */

// ******************************** 0001 1111 ; 0000 0011
// ******************************** 0001 1111 ; 0000 0000
int a = 3; // 00000000 ; 00000000 ; 0000 0000 ; 0000 0011
int b;
char *p;
int c;

int main() {

	printf("a = %x Dir de a: %p \n", a, &a);
	
	/*
	Ojo, un char tiene una logitud de 1 byte, y eso será lo
	máximo de "a" que se podrá manipular mediante "p", a menos
	que se aplique aritmética de punteros.
	*/
	p = (char*) &a;
	
	// Se apunta al segundo byte (se plica aritmética de punteros).
	p = p + 1;

	// Transforma el segundo byte en 1F, por lo que el numero resultante es: 7939 (_10)
	*p = 0x1f;

	printf("a = %x, a = %d. Dir apuntada por p: %p\n", a, a, p);
	
	a = 3; // (_10)
	b = 0x00001f00; // 79936 (_10)
	a = a | b; // 79939 (_10)
	
	printf("a = %x, a = %d. Dir apuntada por p: %p\n", a, a, p);

}
