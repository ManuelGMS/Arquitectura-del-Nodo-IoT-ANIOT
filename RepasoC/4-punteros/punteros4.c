/********* TAREAS ************
 * Compila y ejecuta el código. Responde a las siguientes cuestiones:
 +
 + gcc punteros4.c -o main ; ./main
 +
 * 1. ¿Por qué el segundo printf() imprime un valor diferente para 'd'?
 +
 + Porque "f" dejó de apuntar a suma para pasar a apuntar a resta y, por tanto, esa es la función que se ejecuta.
 +
 * 2. ¿Qué es 'f'? ¿Una variable? ¿Una función? ¿Cómo se "lee" esa línea?
 +
 + La variable "f" es un puntero a una función de parámetros (int, int) y que devuelve un tipo (int).
 +
 * 3. Utiliza la función opera para realizar primero una suma y luego una resta de dos enteros.
 +
 + c = opera(a, b, f = suma);
 + d = opera(a, b, f = resta);
 + printf("c = %d d = %d\n",c,d);
 +
 * 4. Crea un tipo llamado ptrToFunc del mismo prototipo que f usando typedef
 +
 + typedef int (*ptrToFunc)(int a, int b);
 +
 * 5. Crea una función "cambia()" que devuelva un puntero a función que apuntará de forma alterna, a "suma" y  a "resta" en cada invocación que se haga.
 +
 + int counter = 0;
 + int (*cambia()) (int, int) {
 + 		return (counter++ % 2 == 0)? suma : resta;
 + }
 +
 *****************************/

#include <stdio.h>
#include <stdlib.h>

int suma (int x, int y);
int resta(int x, int y);

int suma(int x, int y) { return x + y; }
int resta(int x, int y) { return x - y; }

int opera(int x, int y, int (*g)(int, int)) {
	return g(x,y);
}

int counter = 0;
int (*cambia()) (int, int) {
	return (counter++ % 2 == 0)? suma : resta;
}

int main(void) {

	int a = 12;
	int b =	8;
	int c, d;

	// "f" es un puntero a una funcion de parametros (int, int) y que devuelve (int)
	int (*f)(int a, int b);

	f = suma;
	c = suma(a,b);
	d = f(a,b);
	printf("c = %d d = %d \n",c,d);	
	f = resta;
    d = f(a,b);
	printf("c = %d d = %d \n",c,d);

	// Alternador de funciones.
	f = cambia(a, b);
	c = f(a, b);
	f = cambia(a, b);
	d = f(a, b);
	printf("c = %d d = %d \n",c,d);

}
