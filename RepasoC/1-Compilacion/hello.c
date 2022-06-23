#include <stdio.h>

/******** TAREAS *****
* 1. Compila el siguiente código y ejecútalo (OK)
+
+ gcc hello.c -o hello ; ./hello
+
* 2. Posteriormente, ejecuta solo el pre-procesado y vuelca el resultado a un fichero hello2.i
+
+ gcc hello.c -o hello2.i -E
+
* 3. ¿Qué ha ocurruido con la "llamada a min()"
+
+ Ha sido sustituida por lo especificado en la macro.
+
* 4. ¿Qué efecto ha tenido la directiva #include <stdio.h>?
+
+ Ha sustituido la directiva de la librería por llamadas a funciones externas. Las librerías
+ se añadirán realmente durante la fase de enlazamiento, este puede ser estático o dinámico.
+
*****************/

#define N 5

#define min(x,y) ( (x<y)?x:y )

int a = 7;
int b = 9;

int main() {

 char* cad = "Hello world";
 int i;

 for (i=0;i<N;i++) {
   printf("%s \t a= %d b= %d\n",cad,a,b);
   a++;
   a = min(a,b);			
 }
 return 0;
}
