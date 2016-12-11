/**
 * Krishna Thiyagarajan
 * ECE-357: Operating Systems 
 * Prof. Jeff Hakner
 * Problem Set 6: Semaphores & FIFO
 * December 12, 2016
 * File: fifo.h
 */

#ifndef FIFO_FIFO_H
#define FIFO_FIFO_H

#include "sem.h"

#define MYFIFO_BUFSIZ 4096

struct fifo {
    unsigned long buf[MYFIFO_BUFSIZ];
    int head, tail;
    struct sem rd, wr, access;
};

void fifo_init(struct fifo *f);

void fifo_wr(struct fifo *f, unsigned long d);

unsigned long fifo_rd(struct fifo *f);

#endif //FIFO_FIFO_H
