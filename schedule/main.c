/**
 * Krishna Thiyagarajan
 * Scheduler
 * main.c
 * 12/25/2016
 */

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include "scheduler.h"

#define DELAY_FACTOR 28
#define CHILD_NUM 5

initial() {
    int ii;

    for (ii = 0; ii < CHILD_NUM; ii++) 
        int pid;
        switch (pid = sched_fork()) {
            case -1:
                exit(retWithError("Forking with 'sched_fork()' failed!\n"));
                break;
            case 0:
                sched_nice(ii * 4);
                child();
                break;
        }

    int code;
    for (ii = 0; ii < CHILD_NUM; ii++) {
        sched_wait(&code);
        retWithError("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ PID %d DIED WITH CODE %d $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n", ii + 2,
                     code);
    }

    exit(0);
}


child() {
    int jj;
    retWithError("$$$$$$$$$$$$$$$$$$$$$$$ CHILD PASS 1, PID %d, %p $$$$$$$$$$$$$$$$$$$$$$$$\n", sched_getpid(), &jj);
    for (jj = 1; jj < 1 << DELAY_FACTOR; jj++);
    retWithError("$$$$$$$$$$$$$$$$$$$$$$$ CHILD DONE 1, PID %d, %p $$$$$$$$$$$$$$$$$$$$$$$\n", sched_getpid(), &jj);

    retWithError("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ RESUMING CHILD PID %d $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n", sched_getpid());
    for (jj = 1; jj < 1 << DELAY_FACTOR; jj++);
    retWithError("$$$$$$$$$$$$$$$$$$$$$$$ CHILD DONE 2, PID %d, %p $$$$$$$$$$$$$$$$$$$$$$$\n", sched_getpid(), &jj);
    sched_exit(sched_getpid());
}

main() {
    struct sigaction sa;
    fprintf(stderr, "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ STARTING $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);
    sigaction(SIGABRT, &sa, NULL);
    sched_init(initial);
}
