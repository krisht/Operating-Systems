#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include "sched.h"

#define DELAY_FACTOR 30

initial(){
    int ii;
    switch (sched_fork()){
        case -1:
            return retWithError("Fork 'sched_fork()' failed!\n");
        case 0:
            retWithError("##### \t CHILD PROCESS ADDRESS %p \t #####\n", &ii);
            child1();
            return 0;
        default:
            retWithError("##### \t PARENT PROCESS ADDRESS %p \t #####\n", &ii);
            parent();
            break;
    }
    exit(0);
}

child1(){
    int jj;
    retWithError("$$$$ \t CHILD 1 PASS 1, %p \t $$$$\n", &jj);
    for(jj=1;jj<1<<DELAY_FACTOR;jj++);
    retWithError("$$$$ \t CHILD 1 DONE 1, %p \t $$$$\n", &jj);

    retWithError("$$$$ \t RESUMING CHILD PROCESS 1 \t $$$$\n");
    for(jj=1;jj<1<<DELAY_FACTOR;jj++);
    retWithError("$$$$ \t CHILD 1 DONE 2, %p \t $$$$\n", &jj);
    sched_exit(22);
}

parent(){
    int jj, p;
    retWithError("$$$$ \t IN PARENT PROCESS \t $$$$\n", &jj);
    switch(sched_fork()){
        case -1:
            retWithError("Fork 'sched_fork()' failed!\n");
            return;
        case 0:
            retWithError("##### \t IN CHILD 2 \t #####\n");
            child2();
            sched_exit(11);
            return;
        default:
            retWithError("@@@@@ \t WAITING FOR CHILD \t @@@@@\n");
            while ((p=sched_wait(&jj))>=0)
                retWithError("CHILD PID %d RETURNS WITH CODE %d\n", p, jj);
            return;
    }
}

child2(){
    int jj;
    sched_nice(4);
    retWithError("$$$$ \t CHILD 2 PASS 1, %p \t $$$$\n", &jj);
    for(jj=0;jj<1<<DELAY_FACTOR;jj++);
    retWithError("$$$$ \t CHILD 2 DONE 1, %p \t $$$$\n", &jj);

    retWithError("$$$$ \t RESUMING CHILD PROCESS 2 \t $$$$\n");
    for(jj=0;jj<1<<DELAY_FACTOR;jj++);
    retWithError("$$$$ \t CHILD 2 DONE 2, %p \t $$$$\n", &jj);

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
