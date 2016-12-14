fifoer: fifoer.o sem.o fifo.o taser.o
	gcc -o fifoer.exe fifoer.o sem.o fifo.o tas64.s
	gcc -o taser.exe taser.o tas64.s

fifoer.o: fifoer.c fifo.h
	gcc -c fifoer.c
sem.o: sem.c sem.h
	gcc -c sem.c

fifo.o: fifo.c fifo.h
	gcc -c fifo.c

taser.o: taser.c
	gcc -c taser.c

debug:
	gcc -o fifo_debug.exe -g fifoer.c fifo.c sem.c tas64.s

clean:
	rm *.exe *.o *.stackdump *~