/********* TAREAS ************
 * Compila y ejecuta el código. Responde a las siguientes cuestiones:
 +
 + gcc strings1.c -o main ; ./main
 +
 * 1. El código contiene un error. ¿Se manifiesta en compilación o en ejecución? ¿Por qué se produce?
 * Soluciona el error comentando la(s) línea(s) afectadas. Vuelve a compilar y ejecutar. 
 +
 + Se produce un error en tiempo de ejecución debido a una asignación directa de la variable msg.
 +
 * 2. ¿En qué dirección está la letra 'B' de la cadena Bonjour? ¿Y la de la la letra 'j'?
 +
 + 0x5618d7a9d008 (0; B) y 0x5618d7a9d014 (3; j)  
 +
 * 3. Tras la asignación p=msg2; ¿Cómo podemos recuperar la dirección de la cadena "Bonjour"?
 +
 + 
 +
 * 4. ¿Por qué la longitud de las cadenas p y msg2 es 2 tras la tercera asignación? Se asignan 3 bytes a 'p' que modifican a ambos, pero luego la longitud es sólo 2.
 +
 + Porque se ha introducido la marca \0 en p[2], por lo que strlen solo cuenta los caracteres de p[0] y p[1].
 +
 * 5. ¿Por qué strlen() devuelve un valor diferente a sizeof()?
 +
 + Porque strlen devuelve la cantidad de caracteres que hay hasta toparse con la marca \0
 + Sizeof devuelve la cantidad de memoria en bytes que ocupa un tipo de dato.
 + Ambos casos anteriores no tienen porqué coincidir.
 +
 * 6. ¿Por qué la cadena que se almacena en "msg" en la línea 37 se muestra de forma incorrecta en el printf() final?
 +
 + Porque msg no ha sido inicializada con ningún valor, así que lo que se muestra es la basura que hay en las posiciones que tiene asignada en el STACK.
 +
 ******************************/

#include <stdio.h>
#include <string.h>

int main() {

	void* sp = NULL;
	
	char *p; /* pointer to a char */
	char msg[10]; /* array of 10 chars */
	char msg2[28]="Hello"; /* msg2 = 'H' 'e' 'l' 'l' 'o' '\0' */
     
	p = "Bonjour";
	
	
	printf("STACK pointer address: %p\n", (void*) &sp);
				
	printf("%p ; %p ; %p ; %p ; %p\n", &sp, sp, &p, p, main);
	
	/*
	7 caracteres contiene todo el array de string hasta leer \0
	8 bytes reservados en total, el octavo es donde se ubica \0
	*/
	printf("%ld ; %ld\n", strlen(p), sizeof(p));

	printf("msg: %s, p: %s, msg2: %s\n",msg,p,msg2);
	printf("dir de msg: %p, dir de p: %p, dir de msg2: %p\n",msg,p,msg2);

	p = msg2;
	printf("msg: %s, p: %s, msg2: %s\n",msg,p,msg2);
	printf("dir de msg: %p, dir de p: %p, dir de msg2: %p\n",msg,p,msg2);

 	p[0] = 'H', p[1] = 'i', p[2]='\0';
	printf("msg: %s, p: %s, msg2: %s\n",msg,p,msg2);
	printf("+ msg len: %lu p len %lu msg2 len %lu\n", strlen(msg),strlen(p),strlen(msg2));
	printf("msg size: %lu p size %lu msg2 size %lu\n", sizeof(msg),sizeof(p),sizeof(msg2));

	msg[0] = 'B', msg[1] = 'y';
	printf("msg: %s, p: %s, msg2: %s\n",msg,p,msg2);

	// msg = "Goodbye";
	// printf("msg: %s, p: %s, msg2: %s\n",msg,p,msg2);

}
