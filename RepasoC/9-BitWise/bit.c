#include <stdio.h>

/********* TAREAS ************
 * Compila y ejecuta el código.
 +
 + gcc bit.c -o main ; ./main
 +
 * Estudia la sintaxis de los operadores a nivel de bit y trata de comprender el resultado de cada operación.
 +
 ******************************/

int main() {

	// Formatos de salida.
	printf("%x (HEX) ; %o (OCT) ; %d (DEC)\n", 64, 64, 64);

	int c;
	// N-Bit ***************************************   7  6  5  4 3 2 1 0
	// 2^X = *************************************** 128 64 32 16 8 4 2 1
	int a = 7; // (bin) 00000000 00000000 00000000     0  0  0  0 0 1 1 1
	int b = 9; // (bin) 00000000 00000000 00000000     0  0  0  0 1 0 0 1
	
	c = a & b; // 1 (_10)
	printf("%x AND %x = %x\n", a, b, c);

	c = a | b; // F (_16) ; 15 (_10)
	printf("%x OR %x = %x\n", a, b, c);

	c = a ^ b; // E (_16) ; 14 (_10)
	printf("%x XOR %x = %x\n", a, b, c);

	c = ~a; // FF-FF-FF-F8 (_16)
	printf("NOT %x = %x\n", a, c);

	c = a << 2; // 1C (_16) ; 28 (_10)
	printf("%x << 2 = %x\n", a, c);

	c = a >> 1; // 3 (_16 y _10)
	printf("%x >> 1 = %x\n", a, c);

	// Poner a 0 el bit 2 de a -> pasa de 7 a 3
	c = a & 0xB;
	printf("%x bit 2 a 0 -> %x\n", a, c);

	// Poner a 1 el bit 6 de a -> pasa de 7 a 2^6 + 7 = 71
	c = a | 0x40;
	printf(" %x bit 6 a 1 -> %x\n", a, c);

	// Extraer los bits 2, 3 y 4 de a (a = 7 = 0000111 -> bits 432 =  001)
	c = (a & 0x1C) >> 2;
	printf("bits 4-3-2 de %x: %x\n", a, c);

}
