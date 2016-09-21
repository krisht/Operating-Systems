/**
 * Name:	Krishna Thiyagarajan
 * Class:	ECE-357: Operating Systems
 * P.Set:	#1: System Calls
 */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int bValue = 1024; 			//Default byte value. Changes if -b is specified
char *oValue = "stdout";	//Default output file value. Changes if -o is specified

// Exits program given error description
void exitProgram(const char *func, int errornum, const char *fName = "") {
    fprintf(stderr, "Error in %s('%s') with errno %d: %s.\n", func, fName, errornum, sterror(errornum));
    exit(-1);
}

// Function that syscalls inputFile and writes to output file descriptor
void sysCallFiles(const char *inputFile, int ofd) {
	int ifd = (!strcmp(inputFile, "-")) ? STDIN_FILENO : open(inputFile, O_RDONLY);
    int len, wlen, wRes; 
    char data[bValue];
    
    if (ifd < 0)
        exitProgram("open", errno, inputFile);
    
    while ((len = read(ifd, data, bValue)) > 0 && len != -1)
        if ((wlen = write(ofd, data, len)) < 0)
            exitProgram("write", errno, oValue);
        else if (wlen < len)
            for (int i = wlen; i < len; i++) {
                char newData[1] = {data[i]};
                
                if (write(ofd, newData, 1) < 0)
                    exitProgram("write", errno, oValue);
            }
    
    if (len == -1)
        exitProgram("read", errno, inputFile);
    
    if (ifd != STDIN_FILENO && close(ifd) == -1)
        exitProgram("close", errno, inputFile);
}

// Processes files given the arguments and starting point in argv
void processFiles(int startPoint, int argc, char **argv) {
    int ofd = (!strcmp(oValue, "stdout")) ? STDOUT_FILENO : open(oValue, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    int kk;
    
    if (ofd < 0)
        exitProgram("open", errno, oValue);
    
    if (argc == startPoint)
        sysCallFiles("-", ofd);
    else
        for (kk = startPoint; kk < argc; kk++)
            sysCallFiles(argv[kk], ofd);
    
    if (ofd != STDOUT_FILENO && close(ofd) == -1)
        exitProgram("close", errno, oValue);
}

int main(int argc, char **argv) {
    char ch;

    while ((ch = getopt(argc, argv, "b:o:")) != -1)
        switch (ch) {
            case 'b': 	bValue = atoi(optarg);		break;
            case 'o': 	oValue = optarg;			break;
            case '?':	exit(-1);
            default :	abort();
        }

    processFiles(optind, argc, argv);

    if (close(STDOUT_FILENO))
        exitProgram("close", errno, "stdout");

    exit(0);
}
