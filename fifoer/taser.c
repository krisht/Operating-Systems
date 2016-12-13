/**
 * Krishna Thiyagarajan
 * ECE-357: Operating Systems
 * Prof. Jeff Hakr
 * Problem Set 6: Semaphores & FIFO
 * December 12, 2016
 * File: taser.c
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define N_PROC 10
#define N_REPEATS 1000000

int tas(volatile char *lock);

int main(int argc, char **argv) {

    int *locked = (int *) mmap(NULL, 5, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int *unlocked = (int *) mmap(NULL, 5, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (locked == MAP_FAILED || unlocked == MAP_FAILED) {
        fprintf(stderr, "Error trying to create shared map with code %d: %s\n", errno, strerror(errno));
        exit(-1);
    }

    char *lock = (char *) unlocked++;
    *locked = 0;
    *unlocked = 0;

    int pid, ii;

    for (pid = 0; pid < N_PROC; pid++)
        if (fork() == 0) {
            for (ii = 0; ii < N_REPEATS; ii++)
                (*unlocked)++;
            exit(0);
        }

    while (wait(NULL) > 0);

    *lock = 0;

    for (pid = 0; pid < N_PROC; pid++)
        if (fork() == 0) {
            for (ii = 0; ii < N_REPEATS; ii++) {
                while (tas(lock) == 1);
                (*locked)++;
                *lock = 0;
            }
            exit(0);
        }
    while (wait(NULL) > 0);

    printf("Locked Results: %d/%d\n", *locked, N_PROC * N_REPEATS);
    printf("Unlocked Results: %d/%d\n", *unlocked, N_PROC * N_REPEATS);

    munmap(locked, 5);
    munmap(unlocked, 5);
}