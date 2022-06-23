/********* TAREAS ************
 * Compila y ejecuta el código. Responde a las siguientes cuestiones:
 +
 + gcc punteros3.c -o main ; ./main
 +
 * 1. ¿Por qué cambia el valor de ptr[13] tras la asignación ptr = &c;?
 +
 + Cambia porque lee de otra posicion de memoria diferente y desconocida.
 +
 * 2. El , es muy laxo en ese sentido, es muy laxo en ese sentidoo tiene (al menos) un error. ¿Se manifiesta en compilación o en ejecución? ¿Por qué?
 +
 + Es un error que se da en tiempo de ejecución.
 + Se manifiesta en ejecucion porque en C no se revisa al compilar si se dan o darán posibles lecturas en zonas prohibidas.
 +
 * 3. ¿Qué ocurre con la zona reservada por malloc() tras a asignación ptr = &c;? ¿Cómo se puede acceder a ella? ¿Cómo se puede liberar dicha zona?
 +
 + La zona de memoria reservada en el HEAP es desreferenciada y se pierde, pero la región prevalecerá bloqueada (reservada) y no se podrá liberar 
 + hasta que la RAM se resetee con el apagado o reinicio del computador.
 +
 *****************************/

#include <stdio.h>
#include <stdlib.h>

int nelem;
int c;

int main(void) {
	
	int *ptr;
	int i;

	c = 37;	
	nelem = 127;
	ptr = (int*) malloc(nelem*sizeof(int));
	for (i=0; i<nelem; i++)
		ptr[i] = i;

	printf("ptr[0]= %d ptr[13]=%d \n",ptr[0],ptr[13]);	

	// ptr = &c;
	printf("ptr[0]= %d ptr[13]=%d \n",ptr[0],ptr[13]);	

	free(ptr);
	
}
