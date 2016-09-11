#include <ctype.h>
#include <stdio.h>
#include <err.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h> //contains exit(rval)
#include <unistd.h> //contains getopt

#define DEFAULTBUFFER 1024

char *oValue = NULL;
int bValue = -1;

/**
* Processes files according to description given the following
* @param inputStart start index of the input files
* @param argc       count of arguments supplied
* @param argv       values of the arguments supplied
*/
void processFiles(const int inputStart, int argc, char** argv){

	int ifd, ofd;
    const char* oFileName = (oValue == NULL) ? "stdout" : oValue;
    char* iFileName = (inputStart == argc) ? "stdin" : NULL; 
    char data[(bValue == -1) ? DEFAULTBUFFER : bValue];

    if(iFileName == NULL){
    	
    }


}

int main(int argc, char** argv){

    int ch;
    while((ch = getopt(argc, argv, "b:o:")) != -1)
        switch(ch){
            case 'b': 	bValue = atoi(optarg); 		break;
            case 'o': 	oValue = optarg;			break;
            case '?': 	fprintf(stderr, "usage: copycat [-b ###] [-o outfile] [infile1 infile2 ...]\n");
                exit(1);
                break;
            default : abort();
        }

    processFiles(optind, argc, argv);

    if(fclose(stdout))
        err(1, "stdout");
    exit(0);
}