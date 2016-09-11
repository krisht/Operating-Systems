#include <stdio.h>
#include <err.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

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

    const char* oFileName = (oValue == NULL) ? "stdout" : oValue;
    int ofd = (!strcmp(oFileName, "stdout")) ? fileno(stdout) : open(oFileName, O_WRONLY | O_CREAT | O_APPEND, 0666);
    const char* iFileName = (inputStart == argc) ? "stdin" : "";
    int ifd = (!strcmp(iFileName, "stdin")) ? fileno(stdin) : -1;
    char data[(bValue == -1) ? DEFAULTBUFFER : bValue];

    printf("output: %s\n", oFileName);

    if(ifd != -1)
        printf("input: stdin\n");
    else
        for(int i = inputStart; i < argc; i++){
            if(!strcmp(argv[i], "-"))
            	printf("input: stdin\n");
            else printf("input: %s\n",argv[i]);
        }
}

int main(int argc, char** argv){
    int ch;
    while((ch = getopt(argc, argv, "b:o:")) != -1)
        switch(ch){
            case 'b': 	bValue = atoi(optarg); 		break;
            case 'o': 	oValue = optarg;			break;
            case '?': 	//fprintf(stderr, "usage: copycat [-b ###] [-o outfile] [infile1 infile2 ...]\n");
                		exit(1);
                		break;
            default : abort();
        }

    processFiles(optind, argc, argv);

    if(fclose(stdout))
        err(1, "stdout");
    exit(0);
}