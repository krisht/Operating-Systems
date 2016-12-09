#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include <sys/types.h>


void nocall(){}

int systcall(){
	getuid(); 
}

int main(){
	int repeats = 500000000, ii; 
	struct timespec start; 
	struct timespec end;


	printf("Number of iterations: %d\n", repeats); 
	clock_gettime(CLOCK_REALTIME, &start); 
	for(ii = 0; ii < repeats; ii++); 
	clock_gettime(CLOCK_REALTIME, &end); 
	double diff = (end.tv_sec - start.tv_sec)*1000000000 + (end.tv_nsec - start.tv_nsec);
	printf("Time Per Iteration in an Empty Loop:\t%lf ns.\n", diff/repeats); 

	clock_gettime(CLOCK_REALTIME, &start); 
	for(ii = 0; ii < repeats; ii++)
		nocall();
	clock_gettime(CLOCK_REALTIME, &end); 
	diff = (end.tv_sec - start.tv_sec)*1000000000 + (end.tv_nsec - start.tv_nsec);
	printf("Time Per Iteration in an Function Loop:\t%lf ns.\n", diff/repeats);

	clock_gettime(CLOCK_REALTIME, &start); 
	for(ii = 0; ii < repeats; ii++)
		systcall(); 
	clock_gettime(CLOCK_REALTIME, &end); 
	diff = (end.tv_sec - start.tv_sec)*1000000000 + (end.tv_nsec - start.tv_nsec);
	printf("Time Per Iteration in an SysCall Loop:\t%lf ns.\n", diff/repeats);  



}