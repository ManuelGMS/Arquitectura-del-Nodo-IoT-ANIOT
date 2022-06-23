# Compilador a utilizar
CC = gcc

# Nombre del ejecutable resultante
EXEC = exe

# Argumentos a pasar al compilador
CFLAGS = -Wall -g

# La librería math.h no es enlazada por defecto,
# otras como stdlib.h y stdio.h sí lo són, añadiéndose automáticamente
# la opción -lc. Las dos librerías anteriores tienen implementaciones
# en libc.so para un enlazamiento dinámico y libc.a para un enlazamiento
# dinámico. Hay razones hitóricas para que math.h (libm.so y libm.a) no
# incluyan su funcionalidad en las respectivas libc.so y libc.a
# Algunos compiladores generan un error si -l<library> es pasado como
# argumento al compilador antes de los códigos objetos.
LINK = -lm

# ***************************************************************************

# Las reglas de un Makefile son del tipo	 
# 	<targets> : <prerequisites> 
#		commands
#
# La regla "all" es la que se ejecuta por defecto al hacer "make"
#
all : aux.o init.o
	$(CC) $(CFLAGS) -o $(EXEC) aux.o init.o $(LINK)

# ****************************************************************************

genera : init.o aux.o
	$(CC) $(CFLAGS) -o $(EXEC) init.o aux.o $(LINK)

aux.o : aux.c aux.h
	$(CC) $(CFLAGS) -c aux.c

init.o : init.c
	$(CC) $(CFLAGS) -c init.c

# *****************************************************************************

clean:
	-rm *.o prueba
