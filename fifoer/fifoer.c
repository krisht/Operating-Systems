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
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>

#include "sem.h"
#include "fifo.h"

#define N_PROC 64


struct fifo *map;

void usage() {
    fprintf(stderr, "usage: fifoer [Writers] [Bytes/Writer] [Readers] [Bytes/Reader] [Output]\n");
    exit(EXIT_FAILURE);
}

void exitWithError(const char *format, ...) {
    va_list arg;
    va_start (arg, format);
    vfprintf(stderr, format, arg);
    va_end(arg);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {

    if (argc < 6)
        usage();

    int numWriters = atoi(argv[1]);
    int wBytes = atoi(argv[2]);

    int numReaders = atoi(argv[3]);
    int rBytes = atoi(argv[4]);

    if (numReaders + numWriters > N_PROC)
        exitWithError("Too many processes were requested!\n"); //Finite number of processes allowed

    map = mmap(NULL, sizeof(*map), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0); //map memory

    if (map == MAP_FAILED)
        exitWithError("Error using map with code %d: %s\n", errno, strerror(errno));

    fifo_init(map);

    FILE* outfp = fopen(argv[5], "w"); 

    int ii, jj, pid;
    unsigned long d;

    for (ii = 1; ii <= numWriters; ii++)
        switch (pid = fork()) {
            case -1:
                exitWithError("Error encountered on fork on writer with code %d: %s\n", errno, strerror(errno));
                break;
            case 0:
                for (jj = 1; jj <= wBytes; jj++) {
                    d = ii * 100 + jj;
                    fifo_wr(map, d);
                    fprintf(outfp, "Writer #%d = %lu\n", ii, d);
                }
                exit(0);
                break;
            default:
                break;
        }

    for (ii = 1; ii <= numReaders; ii++)
        switch (pid = fork()) {
            case -1:
                exitWithError("Error encountered on fork on reader with code %d: %s\n", errno, strerror(errno));
                break;
            case 0:
                for (jj = 0; jj < rBytes; jj++) {
                    d = fifo_rd(map);
                    fprintf(outfp, "\t\t\tReader #%d = %lu\n", ii, d);
                }
                exit(0);
                break;
            default:
                break;
        }

    int stat;
    for (ii = 0; ii < numReaders + numWriters; ii++)
        if (wait(&stat) == -1)
            exitWithError("Error returning from child process with code %d: %s", errno, strerror(errno));

    return 0;
}