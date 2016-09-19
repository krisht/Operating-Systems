#include <stdio.h>
#include <err.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int bValue = 1024; //Default byte value. Changes if -b is specified
char* oValue = NULL;  //Default output file value. Changes if -o is specified

/**
 * Exits program given error description
 * @param place    Function where error occurred
 * @param errornum Error number describing the problem
 * @param fName    Optional filename
 */
void exitProgram(const char *place, int errornum,  const char *fName="") {
    fprintf(stderr, "Error in %s('%s') with err %d: %s.\n", place, fName, errornum, strerror(errornum));
    exit(-1);
}

/**
 * Function that syscalls inputFile and writes to output file descriptor
 * @param inputFile Input file to read from
 * @param ofd       Output file descriptor
 */
void sysCallFiles(const char *inputFile, int ofd) {
    int len, wlen, wRes, ifd = (!strcmp(inputFile, "-")) ? STDIN_FILENO : open(inputFile, O_RDONLY);
    char data[bValue];
    if (ifd < 0)
        exitProgram("open", errno, inputFile);
    while ((len = read(ifd, data, bValue)) > 0 && len != -1)
        if((wlen = write(ofd, data, len))< 0)
        	if(wlen < len)
        		exitProgram("write", errno); 
        	else exitProgram("write", errno);
    if(len == -1)
        exitProgram("read", errno,  inputFile);
    if(ifd != STDIN_FILENO && close(ifd) == -1)
        exitProgram("close", errno,  inputFile);
}

/**
 * Processes files given the arguments and starting point in argv
 * @param inputStart Starting point in argv
 * @param argc       Number of arguments from main
 * @param argv       Value of arguments from main
 */
void processFiles(int inputStart, int argc, char **argv) {
    int kk, ofd = (oValue == NULL) ? STDOUT_FILENO : open(oValue, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (ofd < 0)
        exitProgram("open", errno,  oValue);
    if (argc == inputStart)
        sysCallFiles("-", ofd);
    else
        for (kk = inputStart; 	kk < argc; kk++)
            sysCallFiles(argv[kk], ofd);
    if (ofd != STDOUT_FILENO && close(ofd) == -1)
        exitProgram("close", errno,  oValue);
}

int main(int argc, char **argv) {
    int ch;
    while ((ch = getopt(argc, argv, "b:o:")) != -1)
        switch (ch) {
            case 'b':	bValue = atoi(optarg);	break;
            case 'o':	oValue = optarg;		break;
            case '?':	exit(-1);
            default :	abort();
        }
    processFiles(optind, argc, argv);
    if (close(STDOUT_FILENO))
        exitProgram("close", errno, "stdout"); 
    return 0; 
}
