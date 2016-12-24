#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include "scheduler.h"
#include "adjstack.c"

#define STACK_SIZE 65536

int retWithError(const char *format, ...) {
    va_list arg;
    va_start (arg, format);
    vfprintf(stderr, format, arg);
    va_end(arg);
    return -1;
}

void sched_init(void (*init_fn)()){
	resched = 0;
	numPID = 0; 
	numTicks = 0;

	signal(SIGVTALRM, sched_tick);
	signal(SIGABRT, sched_ps);

	struct timeval time;
	time.tv_sec = 0;
	time.tv_usec = 100000; 

	struct itimerval timer;
	timer.it_interval = time;
	timer.it_value = time;

	setitimer(ITIMER_VIRTUAL, &timer, NULL);

	if(!(running = malloc(sizeof(struct sched_waitq))))
		exit(retWithError("Error malloc-ing for running queue with code %d: %s\n",errno, strerror(errno))); 

	if(!(currProc = malloc(sizeof(struct sched_proc))))
		exit(retWithError("Error malloc-ing for current processes with code %d: %s\n", errno, strerror(errno)));

	void *sp;
	if ((sp=mmap(0,STACK_SIZE,PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS,0,0))==MAP_FAILED)
		exit(retWithError("Error mmap-ing for initial stack with error code %d: %s\n", errno, strerror(errno))); 

	int initPid = getNewPid();
	if (initPid < 0)
		exit(retWithError("Error allocationg PID for child process with code %d: %s\n", errno, strerror(errno))); 

	//Create initial process
	struct sched_proc *initProc;
	if(!(initProc = malloc(sizeof(struct sched_proc))))
		exit(retWithError("Error malloc-ing space for initial process with code %d: %s\n", errno, strerror(errno))); 

	initProc->pid =  initPid;
	initProc->ppid = initPid;
	initProc->sp = sp;					
	initProc->state = SCHED_RUNNING;			
	initProc->priority = DEFAULT_PR;
	initProc->timestart = 0;
	initProc->ticksCPU = 0;
	initProc->exitcode = 0;
	initProc->parent = initProc;
	initProc->niceval = 0;

	running->procQueue[0] = initProc;
	running->numprocs = 1;
	currProc = initProc;

	struct savectx currentctx; 

	currentctx.regs[JB_BP] = sp + STACK_SIZE;
	currentctx.regs[JB_SP] = sp + STACK_SIZE;
	currentctx.regs[JB_PC] = init_fn;		//return address in %edx

	restorectx(&currentctx, 0);

	sched_ps();
}

pid_t getNewPid(){
	int ii;
	for(ii = 0; ii < SCHED_NPROC; ii++)
		if(!(running->procQueue[ii])){
			numPID++;
			return ii+1;
		}
}

int sched_fork(){
	
	sigset_t sigset;
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGVTALRM);
	sigprocmask(SIG_BLOCK, &sigset, NULL);
	
	struct sched_proc *childProc;
	
	if(!(childProc = malloc(sizeof(struct sched_proc))))
		return retWithError("Error with malloc-ing space for child process!\n");

	int childPid = getNewPid();
	if(childPid<0)
		return retWithError("Unable to allocate a pid for the child!\n"); 
	
	void *sp;
	if((sp = mmap(0,STACK_SIZE, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, 0 , 0)) == MAP_FAILED)
		return retWithError("Error mmapping child stack with code %d: %s\n", errno, strerror(errno)); 

	memcpy(sp, currProc->sp, STACK_SIZE);

	childProc->pid = childPid;
	childProc->ppid = currProc->pid;
	childProc->sp = sp;
	childProc->state = SCHED_READY;
	childProc->priority = DEFAULT_PR;
	childProc->timestart = currProc->timestart;
	childProc->ticksCPU = currProc->ticksCPU;
	childProc->exitcode = 0;
	childProc->parent = currProc;
	childProc->niceval = 0;
	running->procQueue[childPid-1] = childProc;
	running->numprocs++;

	resched = 1;
	int retval = childPid;
	if(!savectx(&childProc->ctx)){
		unsigned long long adj = childProc->sp - currProc->sp;
		adjstack(sp, sp+STACK_SIZE, adj);
		childProc->ctx.regs[JB_BP] += adj;
		childProc->ctx.regs[JB_SP] += adj;
	}
	else {
		sigprocmask(SIG_UNBLOCK, &sigset, NULL);
		retval = 0;
	}

	sigprocmask(SIG_UNBLOCK, &sigset, NULL);
	return retval;
}

void sched_exit(int code){
	sigset_t sigset;
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGVTALRM);
	sigprocmask(SIG_BLOCK, &sigset, NULL);
	currProc->state = SCHED_ZOMBIE;
	resched = 1;
	currProc->exitcode = code;
	running->numprocs--;

	if(currProc->parent->state ==SCHED_SLEEPING)
		currProc->parent->state = SCHED_READY;

	sigprocmask(SIG_UNBLOCK, &sigset, NULL);
	sched_switch();
};

int sched_wait(int *code){
	sigset_t sigset;
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGVTALRM);
	sigprocmask(SIG_BLOCK, &sigset, NULL);

	int ii;
	int flag = 0;		//flag to show if a child process exists

	for(ii = 0; ii < SCHED_NPROC; ii++)
		if((running->procQueue[ii]) && (running->procQueue[ii]->ppid == currProc->pid))
			//check if child isn't the same as the parent
			if(currProc->pid != running->procQueue[ii]->pid)
				flag = 1;

	if(!flag){
		sigprocmask(SIG_UNBLOCK, &sigset, NULL);
		return -1;
	}

	flag = 0;			//now used to show presence of zombie that belongs to currProc
	while(1){
		for(ii = 0; ii < SCHED_NPROC; ii ++)
			if((running->procQueue[ii]) && (running->procQueue[ii]->state== SCHED_ZOMBIE))
				if(running->procQueue[ii]->ppid == currProc->pid){
					flag = 1;
					break;
				}

		if(flag)
			break;


		currProc->state = SCHED_SLEEPING;
		resched = 1;
		sigprocmask(SIG_UNBLOCK, &sigset, NULL);
		sched_switch();
	}

	*code = running->procQueue[ii]->exitcode;
	free(running->procQueue[ii]);
	running->procQueue[ii] = NULL;
	sigprocmask(SIG_UNBLOCK, &sigset, NULL);
	return 0;
}

void sched_nice(int nice){
	sigset_t sigset;
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGVTALRM);
	sigprocmask(SIG_BLOCK, &sigset, NULL);

	if(nice > 19)
		nice = 19;
	else if (nice < -20)
		nice = 20;
	currProc->niceval = nice;
	sigprocmask(SIG_UNBLOCK, &sigset, NULL);
}

int sched_getpid(){
	return currProc->pid;
}

int sched_getppid(){
	return currProc->ppid;
}

int sched_gettick(){
	return numTicks;
}

void sched_ps() {
	printf("PID     PPID    STATE   STACK           NICE    DYNAMIC   TIME \n");
	int ii;
	for (ii = 0; ii < SCHED_NPROC; ii++)
		if (running->procQueue[ii]){
			printf("%d\t", running->procQueue[ii]->pid);
			printf("%d\t", running->procQueue[ii]->ppid);
			printf("%d\t", running->procQueue[ii]->state);
			printf("%x\t", running->procQueue[ii]->sp);
			printf("%d\t", running->procQueue[ii]->niceval);
			printf("%d\t", running->procQueue[ii]->priority);
			printf("%d\n", running->procQueue[ii]->timestart);
		}
}

void update_priorities(){
	int ii;
	for(ii = 0; ii < SCHED_NPROC;ii++)
		if(running->procQueue[ii]){
			int priority = 20 - running->procQueue[ii]->niceval;
			//process priority heavily weighted against their virtual runtime
            priority -= (running->procQueue[ii]->ticksCPU*8/(20 - running->procQueue[ii]->niceval));
            running->procQueue[ii]->priority = priority;			
		}
}

void sched_switch(){
	sigset_t sigset;
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGVTALRM);
	sigprocmask(SIG_BLOCK, &sigset, NULL);
	update_priorities();
	if(currProc->state == SCHED_RUNNING)
		currProc->state = SCHED_READY;
	
	int ii = 0;
	int procChoice = 0;
	int choicePriority = INT_MIN;
	
	for(ii = 0; ii < SCHED_NPROC; ii++)
		if((running->procQueue[ii] != NULL))
			if((running->procQueue[ii]->state == SCHED_READY)) 
				if((running->procQueue[ii]->priority) > choicePriority){
					procChoice = ii;
					choicePriority = running->procQueue[ii]->priority;
				}

	if((running->procQueue[procChoice]->pid == currProc->pid))
		if((currProc->state == SCHED_READY)){
			// no task switch
			currProc->state = SCHED_RUNNING;
			sigprocmask(SIG_UNBLOCK, &sigset, NULL);
			return;
		}

	if(savectx(&(currProc->ctx)) == 0){
		currProc->ticksCPU = 0;
		currProc = running->procQueue[procChoice];
		fprintf(stderr, "Switched to pid %d\n", currProc->pid);
		sched_ps();
		sigprocmask(SIG_UNBLOCK, &sigset, NULL);
		restorectx(&(currProc->ctx),1);
	}
}

void sched_tick(){
	sigset_t sigset;
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGVTALRM);
	sigprocmask(SIG_BLOCK, &sigset, NULL);
	numTicks++;
	currProc->timestart++;
	currProc->ticksCPU++;

	sigprocmask(SIG_UNBLOCK, &sigset, NULL);
	//leave the decision to take currProc process off to switch()
	sched_switch();
}