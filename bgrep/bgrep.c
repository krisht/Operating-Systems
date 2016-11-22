/**
 * Krishna Thiyagarajan
 * ECE-357: Operating Systems
 * Professor Jeff Hakner
 * bgrep.c - An implementation of a grep function for binary patterns
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <setjmp.h>
#include <stdarg.h>

int pFlag;
int cValue = 0;
int pfdValue;
char *pValue = NULL;
jmp_buf env;
int numPrinted = 0;

int retWithError(const char *format, ...) {
    va_list arg;
    va_start (arg, format);
    vfprintf(stderr, format, arg);
    va_end(arg);
    return -1;
}

void usage(void) {
    retWithError("bgrep {OPTIONS} -currFile pValue_file {file1 {file2} ...}\nbgrep {OPTIONS} pValue {file1 {file2} ...}\n");
    exit(-1); 
}

void sigbusHandler() {
    fprintf(stderr, "Bus error (bad memory access) signal received.");
    longjmp(env, 0);
}

void printInfo(char *printstr, int numchars, char *file) {
    numPrinted = 1;
    printf("%s:", file);
    int jj = 0;
    for (jj = 0; jj < numchars; jj++)
        if (printstr[jj] != '\0')
            if (printstr[jj] > 32 || printstr[jj] == ' ')
                printf("%c", printstr[jj]);
            else
                printf("?");
    printf("\n");
    return;
}


int find(int cValue, char *pValue, char **docs, int numDocs, int isStdin) {

    int fd, ret = 0;
    int loop1=0; 
    struct stat fileStruct;

    char *currFile;
    int fileLoop = 0;
    int kk = 0;

    if (isStdin) {
        numDocs = 1;
        kk = 0;
        currFile = docs[0];
        docs[0] = (char *) "stdin";
    }
    numPrinted = 0;

    for (; kk < numDocs; kk++) {
    	loop1 = 0; 
        fileLoop = 0;

        if(strcmp(docs[kk], "stdin")){
            if (lstat(docs[kk], &fileStruct) != 0) {
            	fprintf(stderr, "Failed to assign fileStruct on %s: %s\n", docs[kk], strerror(errno)); 
            	return -1; 
            }
            fd = open(docs[kk], O_RDONLY);
            currFile = (char *) mmap(NULL, fileStruct.st_size, PROT_READ, MAP_SHARED, fd, 0);
        }

        int stdinCond =  (isStdin == 1) ? 1024 : fileStruct.st_size; //Reads only 1024 bytes from stdin

        for (; fileLoop < stdinCond; fileLoop++) {
            if (setjmp(env) != 0) {
                ret = -1;
                continue;
            }
            if (!currFile[fileLoop]) {
                kk++;
                break;
            }
            if (currFile[fileLoop] == pValue[0]) {
                int cnt = 0;
                int numlooped = 1;
                if (strlen(pValue) == 1) {
                    if (fileLoop - cValue - 1 < 0) {
                        printInfo(&currFile[0], (cValue + strlen(pValue)), docs[kk]);
                        fileLoop++;
                        continue;
                    } else {
                        printInfo(&currFile[fileLoop - cValue], 2 * cValue + strlen(pValue), docs[kk]);
                        fileLoop++;
                        continue;
                    }
                } else {

                    while (++fileLoop < stdinCond && cnt < strlen(pValue) && currFile[fileLoop] == pValue[++cnt]) {
                        if (setjmp(env) != 0) {
                            ret = -1;
                            kk++;
                            loop1 = 1;
                            break;
                        }
                        numlooped++;
                        if (numlooped == strlen(pValue) - 1) {
                            if (fileLoop - cValue < 1)
                                printInfo(&currFile[0], (2 * cValue + strlen(pValue) + (fileLoop - 2 - cValue) - 1), docs[kk]);
                            else
                                printInfo(&currFile[fileLoop - cValue - strlen(pValue) + 2], 2 * cValue + strlen(pValue), docs[kk]);
                            continue;
                        }
                    }

                    if(loop1)
                    	break; 
                }
            }
        }
    }

    if (ret == -1)
        return -1; 
    if (numPrinted != 0)
        return 0;
    return 1;
}

int main(int argc, char **argv) {

    signal(SIGBUS, sigbusHandler);

    pValue = (char *) malloc(sizeof(char) * (2049));
    struct stat fileStruct;
    struct stat stdinStruct;
    int redir = 0;
    int isStdin = 0;

    char ch;
    while ((ch = getopt(argc, argv, "c:p:")) != -1)
        switch (ch) {
            case 'c':
                cValue = atoi(optarg);
                if (strcmp(optarg, "0") && cValue == 0)
                    return retWithError("Invalid argument after -c: Integer expected!\n", optopt);
                break;

            case 'p':
                if (strlen(optarg) >= 2048)
                    return retWithError("Invalid argument after -p: Pattern is more than 2048 characters!\n", optopt);

                pfdValue = open(optarg, O_RDONLY);

                if (lstat(optarg, &fileStruct) != 0)
                    return retWithError("Failed to get struct on argument -p %s: %s\n", optarg, strerror(errno));

                pValue = (char *) mmap(NULL, fileStruct.st_size, PROT_READ, MAP_SHARED, pfdValue, 0);
                pFlag = 1;
                break;
            case ':':
                return retWithError("Argument expected after -%c\n", optopt);
                break;
            case '?':
                return retWithError("Unknown flag detected!\n");
            default :
                usage();
        }

    char **docs = (char **) malloc(sizeof(char) * (argc - optind) * 2049);

    if (!pFlag)
        pValue = argv[optind];

    int kk;
    for (kk = 0; kk < argc - optind; kk++) {
        if (!pFlag) {
            pFlag = 2;
            continue;
        }
        int index = pFlag == 2 ? kk -1 : kk; 
        docs[index] = argv[optind + kk];
    }

    if ((pFlag == 1 && argc == optind) || (pFlag == 2 && argc - 1 == optind)) {
        redir = 1;
        if (fstat(0, &stdinStruct) != 0)
            return retWithError("Failed to get fileStruct from stdin. Error %d: %s", errno, strerror(errno));
        docs[0] = (char *) mmap(NULL, stdinStruct.st_size, PROT_READ, MAP_SHARED, 0, 0);
        isStdin = 1;
    }

    int tempNumDocs = (pFlag == 2) ? kk - 1 : kk; 

    int numDocs = redir == 1 ? 1 : tempNumDocs;

    return find(cValue, pValue, docs, numDocs, isStdin);
}
