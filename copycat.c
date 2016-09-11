
#include <iostream>

#include <ctype.h>
#include <stdio.h>
#include <err.h>
#include <stdlib.h> //contains exit(rval)
#include <unistd.h> //contains getopt
using namespace std;

int bFlag, oFlag;
char *oValue = NULL;
int bValue = 0; 
int rval; 

static void usage(void);
void processFiles(const int inputStart, int argc, char** argv); 

int main(int argc, char** argv){

	int ch; 

	opterr = 0; 

	while((ch=getopt(argc, argv, "b:o:")) != -1){
		switch(ch){
			case 'b': 	bFlag=1; 
						bValue = atoi(optarg);
						break;
			case 'o': 	oFlag = 1;
						oValue = optarg; 
						break;
			case '?':	usage(); 
						exit(1); 
						break; 
			default : abort(); 
		}

	}

	processFiles(optind, argc, argv); 

	if(fclose(stdout))
		err(1, "stdout"); 
	exit(rval); 
}

void processFiles(const int inputStart, int argc, char** argv){
/*	
	const char* oFileName = oValue; 
	char* iFileName; 
	int ifd, ofd; 


	if(inputStart == argc){
		oFileName = "stdout"; 
		ifd = STDIN_FILENO;
	}*/

	printf("B Value: %d\n",  bValue); 
	printf("O Value: %s\n", oValue); 

	for(int i = inputStart; i < argc; i++)
		printf("Input: %s\n", argv[i]); 
}

static void
usage(void)
{
	fprintf(stderr, "usage: copyccat [-b ###] [-o outfile] [infile1 infile2 ...]\n");
	exit(1);
}
