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

#include "sem.h"

void sigusrhandler(int sig) {
    return;
}

void sem_init(struct sem *s, int count) {
    s->count = count;
    s->lock = 0;

    int ii;
    for (ii = 0; ii < N_PROC; ii++) {
        s->proc_status[ii] = 0;
        s->procID[ii] = 0;
    }
}

int sem_try(struct sem *s) {
    while (tas(&(s->lock)));
    if (s->count > 0) {
        s->count--;
        s->lock = 0;
        return 1;
    }

    s->lock = 0;
    return 0;
}

void sem_wait(struct sem *s) {
    s->procID[proc_num] = getpid();
    while (1) {
        while (tas(&(s->lock)));
        signal(SIGUSR1, sigusrhandler);
        sigset_t mask, omask;
        sigfillset(&mask);
        sigdelset(&mask, SIGINT);
        sigdelset(&mask, SIGUSR1);
        sigprocmask(SIG_BLOCK, &mask, &omask);

        if (s->count > 0) {
            s->count--;
            s->proc_status[proc_num] = 1;
            sigsuspend(&mask);
        }

        sigprocmask(SIG_UNBLOCK, &mask, NULL);

    }
}

void sem_inc(struct sem *s) {
    while (tas(&(s->lock)));
    s->count++;

    int ii;
    for (ii = 0; ii < N_PROC; ii++)
        if (s->proc_status[ii])
            kill(s->procID[ii], SIGUSR1);

    s->lock = 0;
}