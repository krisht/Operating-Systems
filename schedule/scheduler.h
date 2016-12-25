/**
 * Krishna Thiyagarajan
 * Scheduler
 * scheduler.h
 * 12/25/2016
 */


#ifndef SCHEDULER_H
#define SCHEDULER_H

#define SCHED_NPROC  256
#define SCHED_READY 0
#define SCHED_RUNNING 1
#define SCHED_SLEEPING 2
#define SCHED_ZOMBIE 3

#define DEFAULT_PR 20

#include <stdarg.h>
#include "jmpbuf-offsets64.h"

struct savectx {
    void *regs[JB_SIZE];
};

struct sched_proc {
    int pid;
    int ppid;
    void *sp;
    int state;
    int priority;
    int procTime;
    int cpuTime;
    int exitcode;
    int niceval;
    int lastTick;
    struct savectx ctx;
    struct sched_proc *parent;
};


struct sched_waitq {
    struct sched_proc *procQueue[SCHED_NPROC];
    int countProcs;
};

struct sched_proc *currProc;
struct sched_waitq *running;

int numPid, numTicks;
int resched;

int retWithError(const char *format, ...);

void sched_init(void (*init_fn)());

int sched_fork();

void sched_exit(int ecode);

int sched_wait(int *ecode);

void sched_nice(int niceval);

int sched_getpid();

int sched_getppid();

int sched_gettick();

void sched_ps();

void sched_switch();

void sched_tick();

#endif