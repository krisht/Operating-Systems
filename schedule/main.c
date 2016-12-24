#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include "sched.h"

#define DELAY_FACTOR 30
#define CHILD_NUM 5

initial(){
    int ii;

    for( ii = 0; ii < CHILD_NUM; ii++){
        int pid; 
        switch(pid = sched_fork()){
            case -1: 
                retWithError("Forking with 'sched_fork()' failed!\n");
                exit(-1);
                break; 
            case 0:
                sched_nice(ii*4); 
                child(); 
                break; 
        }
    }
    int code; 
    for(ii = 0; ii < CHILD_NUM; ii++)
        sched_wait(&code); 

    exit(0);
}

child(){
    int jj;
    retWithError("$$$$ \t CHILD PASS 1, %p \t $$$$\n", &jj);
    for(jj=1;jj<1<<DELAY_FACTOR;jj++);
    retWithError("$$$$ \t CHILD DONE 1, %p \t $$$$\n", &jj);

    retWithError("$$$$ \t RESUMING CHILD PROCESS \t $$$$\n");
    for(jj=1;jj<1<<DELAY_FACTOR;jj++);
    retWithError("$$$$ \t CHILD DONE 2, %p \t $$$$\n", &jj);
    sched_exit(22);
}

main(){
    struct sigaction sa;
    fprintf(stderr,"!!!!! \t STARTING \t !!!!!\n");
    sa.sa_flags=0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGUSR1,&sa,NULL);
    sigaction(SIGUSR2,&sa,NULL);
    sigaction(SIGABRT,&sa,NULL);
    sched_init(initial);
}
