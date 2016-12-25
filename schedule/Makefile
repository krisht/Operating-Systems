main.exe: main.o scheduler.o savectx64.S
	gcc -g -o main.exe main.c scheduler.c savectx64.S

main.o: main.c
	gcc -c main.c

scheduler.o: scheduler.c scheduler.h jmpbuf-offsets64.h
	gcc -c scheduler.c

debug: 
	gcc -g -o schedDebug.exe main.c  scheduler.c adjstack.c savectx64.S

clean:
	rm -f *.exe *.o *.stackdump *~

backup:
	test -d backups || mkdir backups
	cp *.c backups
	cp *.h backups
	cp Makefile backups