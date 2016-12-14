/**
 * Krishna Thiyagarajan
 * ECE-357: Operating Systems
 * Prof. Jeff Hakner
 * Problem Set 6: Semaphores & FIFO
 * December 12, 2016
 * File: fifoer.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdarg.h>

#include "sem.h"
#include "fifo.h"

#define N_PROC 64


void usage() {
    exitWithError("Usage: fifoer [Writers] [Bytes/Writer] [Readers] [Bytes/Reader]\n");
}

int main(int argc, char **argv) {

    if (argc < 5)
        usage();

    int numWriters = atoi(argv[1]);
    int wBytes = atoi(argv[2]);

    int numReaders = atoi(argv[3]);
    int rBytes = atoi(argv[4]);

    if (numReaders + numWriters > N_PROC)
        exitWithError("Too many processes were requested!\n"); //Finite number of processes allowed

    struct fifo* map = mmap(NULL, sizeof(*map), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0); //map memory

    if (map == MAP_FAILED)
        exitWithError("Error using map with code %d: %s\n", errno, strerror(errno));

    fifo_init(map);

    int ii, jj, pid;
    unsigned long d;

    for (ii = 1; ii <= numReaders; ii++)
        switch (pid = fork()) {
            case -1:
                exitWithError("Error encountered on fork on reader with code %d: %s\n", errno, strerror(errno));
            case 0:
                for (jj = 0; jj < rBytes; jj++) {
                    d = fifo_rd(map);
                    printf("\t\t\tR-%d = %lu\n", ii, d);
                }
                exit(0);
            default:
                continue;
        }

    for (ii = 1; ii <= numWriters; ii++)
        switch (pid = fork()) {
            case -1:
                exitWithError("Error encountered on fork on writer with code %d: %s\n", errno, strerror(errno));
            case 0:
                for (jj = 1; jj <= wBytes; jj++) {
                    d = ii * 100 + jj;
                    fifo_wr(map, d);
                    printf("W-%d = %lu\n", ii, d);
                }
                exit(0);
            default:
                continue;
        }

    int stat;
    for (ii = 0; ii < numReaders + numWriters; ii++)
        if (wait(&stat) == -1)
            exitWithError("Error returning from child process with code %d: %s", errno, strerror(errno));

    exit(0);
}


