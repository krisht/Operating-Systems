#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int realTimeSigCount=0, regTimeSigCount=0; 
int realTimeSig, regTimeSig;

void handleReal(){ realTimeSigCount++; }

void handleReg(){ regTimeSigCount++; }

int generateSignals(){
	int ppid, kk, jj, wstat; 
	int pid[10]; 

	for(jj = 0; jj < 10; jj++)
		switch(pid[jj] = fork()){
			case -1:
				fprintf(stderr, "ERROR fork() failed: %s\n", strerror(errno)); 
			case 0:
				ppid = getppid(); 
				for(kk = 0; kk < 1000; kk++){
					kill(ppid, regTimeSig); 
					kill(ppid, realTimeSig); 
				}
				exit(0);
			default: 
				continue; 
		}

	while(jj-- > 0)
		wait(&wstat);
	return 0;
}

int main(int argc, char** argv){
	realTimeSig = SIGRTMIN;
	regTimeSig = SIGSEGV; 

	signal(realTimeSig, handleReal); 
	signal(regTimeSig, handleReg); 

	generateSignals(); 

	printf ("%d \treal-time signals delivered\n", realTimeSigCount); 
	printf("%d \tconventional signals delivered\n", regTimeSigCount); 
}