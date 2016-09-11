#include <stdio.h>
#include <err.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULTBUFFER 1024

char *oValue = NULL;
int bValue = -1;

void sysCallFiles(const char* inputFile, int ofd ){
/*    char data[(bValue == -1) ? DEFAULTBUFFER : bValue];
    int fd = open(inputFile, O_RDONLY);
    read(fd, data); // incorrect code
    close(fd);*/
}

void processFiles(const int inputStart, int argc, char** argv){

    const char* oFileName = (oValue == NULL) ? "stdout" : oValue;
    int ofd = (!strcmp(oFileName, "stdout")) ? fileno(stdout) : open(oFileName, O_WRONLY | O_CREAT | O_APPEND, 0666);
    const char* iFileName = (inputStart == argc) ? "stdin" : "";

    printf("output: %s\n", oFileName);

    if(!strcmp(iFileName, "stdin")){
        printf("input: stdin\n");
        sysCallFiles("stdin", ofd);
    }
    else
        for(int i = inputStart; i < argc; i++){
            if(!strcmp(argv[i], "-")){
                printf("input: stdin\n");
                sysCallFiles("stdin", ofd);
            }
            else{
                printf("input: %s\n",argv[i]);
                sysCallFiles(argv[i], ofd);
            }
        }
}

int main(int argc, char** argv){
    int ch;
    while((ch = getopt(argc, argv, "b:o:")) != -1)
        switch(ch){
            case 'b': 	bValue = atoi(optarg); 		break;
            case 'o': 	oValue = optarg;			break;
            case '?': 	exit(1); //fprintf(stderr, "usage: copycat [-b ###] [-o outfile] [infile1 infile2 ...]\n");
            default : abort();
        }

    processFiles(optind, argc, argv);

    if(fclose(stdout))
        err(1, "stdout");
    exit(0);
}