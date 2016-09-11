#include <stdio.h>
#include <err.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define DFLT_BUF_SIZE 1024

char *oValue = NULL;
int bValue = -1;

void sysCallFiles(const char *inputFile, int ofd) {
    int ifd = (!strcmp(inputFile, "stdin")) ? fileno(stdin) : open(inputFile, O_RDONLY);
    int len;
    if (bValue == -1) {
        char data[DFLT_BUF_SIZE];
        while ((len = read(ifd, data, DFLT_BUF_SIZE)) > 0)
            write(ofd, data, len);
    }
    else {
        char data[bValue];
        len = read(ifd, data, bValue);
        write(ofd, data, len);
    }
    write(ofd, "\n", 1);
    close(ifd);

}

void processFiles(int inputStart, int argc, char **argv) {

    const char *oFileName = (oValue == NULL) ? "stdout" : oValue;
    int ofd = (!strcmp(oFileName, "stdout")) ? fileno(stdout) : open(oFileName, O_WRONLY | O_CREAT | O_APPEND, 0666);
    const char *iFileName = (inputStart == argc) ? "stdin" : "";

    printf("output: %s\n", oFileName);

    if (!strcmp(iFileName, "stdin"))
        sysCallFiles("stdin", ofd);
    else {
        int kk;
        for (kk = inputStart; kk < argc; kk++)
            if (!strcmp(argv[kk], "-"))
                sysCallFiles("stdin", ofd); //why isn't it syscalling again when "-" is indicated
            else sysCallFiles(argv[kk], ofd);
    }
    close(ofd);
}

int main(int argc, char **argv) {
    int ch;
    while ((ch = getopt(argc, argv, "b:o:")) != -1)
        switch (ch) {
            case 'b':
                bValue = atoi(optarg);
                break; //Check whether optarg is a number
            case 'o':
                oValue = optarg;
                break;
            case '?':
                exit(1); //fprintf(stderr, "usage: copycat [-b ###] [-o outfile] [infile1 infile2 ...]\n");
            default :
                abort();
        }

    processFiles(optind, argc, argv);

    if (fclose(stdout))
        err(1, "stdout");
    exit(0);
}