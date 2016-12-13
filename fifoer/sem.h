/**
 * Krishna Thiyagarajan
 * ECE-357: Operating Systems 
 * Prof. Jeff Hakner
 * Problem Set 6: Semaphores & FIFO
 * December 12, 2016
 * File: sem.h
 */

#ifndef FIFO_SEM_H
#define FIFO_SEM_H

#define N_PROC 64

int proc_num;

struct sem {
    volatile char lock;
    int count;
    int proc_status[N_PROC];
    int procID[N_PROC];
};

int tas(volatile char *lock);

void sem_init(struct sem *s, int count);

int sem_try(struct sem *s);

void sem_wait(struct sem *s);

void sem_inc(struct sem *s);

#endif //FIFO_SEM_H