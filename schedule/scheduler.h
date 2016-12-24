#ifndef SCHED_H
#define SCHED_H

#define SCHED_NPROC  256 

#define SCHED_READY 0
#define SCHED_RUNNING 1
#define SCHED_SLEEPING 2
#define SCHED_ZOMBIE 3

#define DEFAULT_PR 20

#include <stdarg.h>

#include "jmpbuf-offsets64.h"


struct savectx{
	void *regs[JB_SIZE];
};

struct sched_proc{
	int pid, ppid; 
	void *sp;			// stack pointer
	int state, priority; // task state & priority
	int timestart; 		// delta t for process
	int ticksCPU;		// number of cpu ticks with process
	int exitcode; 		// exit code/return value of process
	int niceval; 		// nice level of process
	int endtickcount; 	//stores lsat tick count when process is switched
	struct savectx ctx; 
	struct sched_proc *parent; //parent proc
};


struct sched_waitq{
	struct sched_proc *procQueue[SCHED_NPROC];
	int numprocs;
};

struct sched_proc *currProc; //Current process
struct sched_waitq *running; //Running queue

int numPID, numTicks;
int resched;

int retWithError(const char *format, ...); 

void sched_init(void (*init_fn)());

int sched_fork();

void sched_exit(int code);

int sched_wait(int *code);

void sched_nice(int niceval);

int sched_getpid();

int sched_getppid();

int sched_gettick();

void sched_ps();

void sched_switch();

void sched_tick();

#endif