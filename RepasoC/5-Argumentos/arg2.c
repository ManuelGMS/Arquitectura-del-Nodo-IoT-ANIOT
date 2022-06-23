/************* TAREAS
 * Compila el código y ejecútalo. Responde:
 +
 + gcc arg2.c -o main ; ./main
 +
 * 1. ¿Por qué cambia el valor de 'y' tras la llamada a suma()?
 +
 + Como "y" está pasado por referencia se modifica el contenido de la varible original.
 +
 * 2. ¿Por qué en ocasiones se usa el operador '.' y en otras '->' para acceder a los campos de una estructura?
 +
 + La notación '.' la utilizan los struct estáticos, la notación '->' la utilizan los punteros a struc.
 +
 * 3. ¿Por qué el valor de zc pasa a ser incorrecto sin volver a usarlo en el código?
 +
 + 
 +
 * 4. Corrije el código para evitar el error producido en sumC
 +
 + Hay que usar "r" como un puntero a struct _complex_ en el HEAP para que al final de la función pueda devolverse la dirección de "r".
 +
 ********************/

#include <stdio.h>
#include <stdlib.h>

/* Tipo estructurado */
struct _complex_ {
	float re;
	float im;
};

/* Declaración adelantada */
int sum(int *pa, int *pb);
struct _complex_ * sumC( struct _complex_ *a,  struct _complex_ *b);

int main(void) {
	
	int x = 4, y = 5;
	int* ptr = &y;
	struct _complex_ *zc; 
	struct _complex_ xc = {.re = 1.0, .im = 2.0};
	struct _complex_ yc = {.re = 3.0, .im = 1.0};
	
	printf("Suma de complejos. (%f,%f i) + (%f,%f i) = ", xc.re, xc.im, yc.re, yc.im);
	zc = sumC(&xc, &yc);
	printf("(%f,%f i)\n", zc->re, zc->im);
	int total = sum(&x, ptr); 

	printf("Suma de enteros:  x + y = %d + %d = %d \n", x, y, total);
	printf("xc = (%f,%f i)  yc = (%f,%f i) zc = (%f,%f i)\n",xc.re,xc.im,yc.re,yc.im,zc->re,zc->im);
	
	return 0;

}

int sum(int *pa, int *pb) {  
   /* args passed by reference */
  int c = *pa + *pb;
  printf("SE TIENE: %d ; %d\n", *pa, *pb);
  int buf[256] = {0};
  *pa = 7;
  *pb = 8;	
   return c;      /* return by value */
}

struct _complex_ * sumC(struct _complex_* a,  struct _complex_* b) {
	/*
	struct _complex_ r;
	r.re = a->re + b->re;
	r.im = a->im + b->im;
	// Tratamos de modificar el primer argumento
	a->re = 12.5;
	a->im = 13.4;
	return &r;
	*/
	struct _complex_ * r = (struct _complex_ *) malloc(sizeof(struct _complex_));
	r->re = a->re + b->re;
	r->im = a->im + b->im;
	// Tratamos de modificar el primer argumento
	a->re = 12.5;
	a->im = 13.4;
	return r;
}
