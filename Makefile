# Makefile
# Sistemas Operativos, DEI/IST/ULisboa 2017-18

CFLAGS= -g -Wall -pedantic -c -fPIC

heatSim: main.o matrix2d.o barreira.o
	gcc -pthread -o heatSim main.o matrix2d.o barreira.o

main.o: main.c matrix2d.h barreira.h
	gcc $(CFLAGS) main.c

matrix2d.o: matrix2d.c matrix2d.h
	gcc $(CFLAGS)  matrix2d.c

barreira.o: barreira.c barreira.h
	gcc $(CFLAGS) barreira.c

clean:
	rm -f *.o heatSim

clean_zip:
	rm -f *.zip

zip:
	zip proj1.zip main.c barreira.c barreira.h matrix2d.c matrix2d.h Makefile

