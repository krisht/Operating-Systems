/**
 * Krishna Thiyagarajan
 * ECE-357: Operating Systems
 * Prof. Jeff Hakner
 * Problem Set 6: Semaphores & FIFO
 * December 12, 2016
 * File: sem.c
 */

#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdarg.h>

#include "sem.h"

void exitWithError(const char *format, ...) {
    va_list arg;
    va_start (arg, format);
    vfprintf(stderr, format, arg);
    va_end(arg);
    exit(EXIT_FAILURE);
}

void sigusrhandler(int sig) { }

void sem_init(struct sem *s, int count) {
    s->count = count;
    s->lock = 0;

    int ii;
    for (ii = 0; ii < N_PROC; ii++)
        s->proc_status[ii] = s->procID[ii] = 0;
}

int sem_try(struct sem *s) {
    while (tas(&(s->lock)));

    s->lock = 0;

    if (s->count > 0) {
        s->count--;
        return 1;
    }

    return 0;
}

void sem_wait(struct sem *s) {

    s->procID[proc_num] = getpid(); //Set proc flag to indicate wait

    //Loop for waiting
    for (; ;) {

        while (tas(&(s->lock)));

        //Block all signals and handle SIGUSR1
        signal(SIGUSR1, sigusrhandler);

        sigset_t mask, omask;


        if(sigfillset(&mask) != 0)
            exitWithError("Error building new mask with code %d: %s\n", errno, strerror(errno));

        if(sigdelset(&mask, SIGINT) != 0 )
            exitWithError("Error enabling SIGINT with code %d: %s\n", errno, strerror(errno));

        if(sigdelset(&mask, SIGUSR1) != 0)
            exitWithError("Error enabling SIGUSR1 with code %d: %s\n", errno, strerror(errno));

        if(sigprocmask(SIG_BLOCK, &mask, &omask) !=0)
            exitWithError("Error when trying to block with code %d: %s\n", errno, strerror(errno));

        if (s->count > 0) {
            s->count--;
            s->proc_status[proc_num] = 0;
            s->lock = 0;
            return;
        }

        s->lock = 0;
        s->proc_status[proc_num] = 1;
        
        sigsuspend(&mask);

        if(sigprocmask(SIG_UNBLOCK, &mask, NULL) != 0)
            exitWithError("Error when trying to unblock with code %d: %s\n", errno, strerror(errno));

    }
}

void sem_inc(struct sem *s) {
    while (tas(&(s->lock))); //Get a lock

    s->count++;

    //Wake up sleeping processes
    int ii;
    for (ii = 0; ii < N_PROC; ii++)
        if (s->proc_status[ii])
            if(kill(s->procID[ii], SIGUSR1) !=0)
                exitWithError("Error killing process with pid %d with code %d: %s\n", s->procID[ii], errno, strerror(errno));

    s->lock = 0; // Unlock
}
