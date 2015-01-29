#
# Makefile for Amiga (Aztec C and others..)
#

OBJS = 7plus.o decode.o encode.o correct.o rebuild.o extract.o join.o utils.o

7plus.o : 7plus.c 7plus.h
   cc 7plus.c -psa

decode.o : decode.c 7plus.h globals.h
   cc decode.c -psa

encode.o : encode.c 7plus.h globals.h
   cc encode.c -psa

correct.o : correct.c 7plus.h globals.h
   cc correct.c -psa

rebuild.o : rebuild.c 7plus.h globals.h
   cc rebuild.c -psa

extract.o : extract.c 7plus.h globals.h
   cc extract.c -psa

join.o : join.c 7plus.h globals.h
   cc join.c -psa

utils.o : utils.c 7plus.h globals.h
   cc utils.c -psa

# not needed on the Amiga.
# unix.o : unix.c 7plus.h globals.h
#   cc unix.c -psa

7plus: $(OBJS) 7plus
   ln -g -o 7plus $(OBJS) -lc16
