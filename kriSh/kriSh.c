#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <stdarg.h>

#define DEBUG_MODE 1

//Error reporting and exit function
void specialExit(const char *retVal, const char *format, ...) {
    va_list arg;
    va_start (arg, format);
    vfprintf(stderr, format, arg);
    va_end(arg);
    if (!strcmp(retVal, "NULL"))
        return;
    else exit(atoi(retVal));
}

// Gets arguments from input line
char **getArgs(char *line, int *count) {
    char **args = NULL;
    const char *delimiters = "\t ";
    line[strlen(line) - 1] = 0;

    char *arg = strtok(line, delimiters);

    while (arg != NULL) {
        args = (char **) realloc(args, ++(*count) * sizeof(char *));
        if (args == NULL)
            specialExit("1", "Error reallocating args memory.\n");

        args[*count - 1] = arg;
        arg = strtok(NULL, delimiters);
    }
    return args;
}

//Gets number of args without redirections
int getCmdArgC(char **args, int count) {
    int kk;
    int cmdArgC = 0;
    for (kk = 0; kk < count; kk++) {
        char *a = args[kk];
        if (!(a[0] == '<' || a[0] == '>' || (a[0] == '2' && a[1] == '>')))
            cmdArgC++;
    }
    return cmdArgC;
}

// Returns amount of redirection arguments
int getRedirArgC(char **args, int count) {
    int kk;
    int redirArgC = 0;
    for (kk = 0; kk < count; kk++) {
        char *a = args[kk];
        if ((a[0] == '<' || a[0] == '>' || (a[0] == '2' && a[1] == '>')))
            redirArgC++;
    }
    return redirArgC;
}

//Forking function that forks given certain parameters
int forkProcess(int redirArgC, char **redirArgV, char **rawArgV, char **cmdArgV) {
    //Declares timers, resource usages and exit status
    struct timeval startTime, endTime;
    struct rusage resUsage;
    int exitStat;

    if (gettimeofday(&startTime, NULL) < 0)
        specialExit("1", "Error getting start time: %s\n", strerror(errno));

    switch (fork()) {
        case 0: {
            // I/O Redirection
            int kk, fd, fileNum;
            for (kk = 0; kk < redirArgC; kk++) {
                char *fileName = NULL;
                if (redirArgV[kk][0] == '<') { //Redirect STDIN
                    fileName = redirArgV[kk] + 1; //Offset to find filename is 1
                    fd = open(fileName, O_RDONLY);
                    fileNum = STDIN_FILENO;
                } else if (redirArgV[kk][0] == '>' && redirArgV[kk][1] != '>') { // Redirect stdout (open/creat/trunc)
                    fileName = redirArgV[kk] + 1;
                    fd = open(fileName, O_WRONLY | O_TRUNC | O_CREAT, S_IREAD | S_IWRITE);
                    fileNum = STDOUT_FILENO;
                } else if (redirArgV[kk][0] == '2' && redirArgV[kk][1] == '>' && redirArgV[kk][2] != '>') { // Redirect stderr (open/creat/trunc)
                    fileName = redirArgV[kk] + 2; //Offset to find filename is 2
                    fd = open(fileName, O_WRONLY | O_TRUNC | O_CREAT, S_IREAD | S_IWRITE);
                    fileNum = STDERR_FILENO;
                } else if (redirArgV[kk][0] == '>' && redirArgV[kk][1] == '>') {//Redirect stdout (open/creat/append)
                    fileName = redirArgV[kk] + 2;
                    fd = open(fileName, O_WRONLY | O_APPEND | O_CREAT, S_IREAD | S_IWRITE);
                    fileNum = STDOUT_FILENO;
                } else if (redirArgV[kk][0] == '2' && redirArgV[kk][1] == '>' && redirArgV[kk][2] == '>') { //Redirect stderr (open/creat/append)
                    fileName = redirArgV[kk] + 3; //Offset to find filename is 3
                    fd = open(fileName, O_WRONLY | O_APPEND | O_CREAT, S_IREAD | S_IWRITE);
                    fileNum = STDERR_FILENO;
                }

                if (fd < 0) //Exits when error in opening fileName file
                    specialExit("1", "Error with opening file, %s: %s", fileName, strerror(errno));

                if (dup2(fd, fileNum) < 0)
                    specialExit("1", "Error with dup2 %s to fd %d: %s\n", fileName, fileNum, strerror(errno));

                close(fd); //Closes original file descriptor
            }

            execvp(rawArgV[0], cmdArgV); //executes command
            free(rawArgV);
            break;
        }
        case -1:
            specialExit("1", "Fork failed: %s\n", strerror(errno)); //Fork failed. Exit
        default:
            if (wait3(&exitStat, 0, &resUsage) < 0) //Wait for process to change state
                specialExit("1", "Error with wait3: %s\n", strerror(errno));

            if (gettimeofday(&endTime, NULL) < 0) //Find ending time
                specialExit("1", "Error with getting end time: %s\n", strerror(errno));

            double rTimeDiff, uTimeDiff, sTimeDiff;

            //Calculate and print different run times
            rTimeDiff = (((endTime.tv_sec - startTime.tv_sec) * 1000000 + endTime.tv_usec) - startTime.tv_usec) /
                        1000000.0;
            uTimeDiff = (resUsage.ru_utime.tv_sec * 1000000L + resUsage.ru_utime.tv_usec) / 1000000.0;
            sTimeDiff = (resUsage.ru_stime.tv_sec * 1000000L + resUsage.ru_stime.tv_usec) / 1000000.0;

            printf("\n[Finished with exit code %d]\n", WEXITSTATUS(exitStat));
            printf("Real: %03lfs, User: %03lfs, System: %03lfs\n", rTimeDiff, uTimeDiff, sTimeDiff);

    }
}

void execCmd(char *line) {
    int count = 0; //Number of arguments
    char **rawArgV = getArgs(line, &count);
    int cmdArgC = getCmdArgC(rawArgV, count);
    char **cmdArgV = (char **) malloc((cmdArgC + 1) * sizeof(char *));

    if (cmdArgC > 0) {
        int kk;
        for (kk = 0; kk <= cmdArgC; kk++)
            cmdArgV[kk] = rawArgV[kk]; //Add raw arguments to command arguments
        cmdArgV[cmdArgC] = 0; //Add null terminator
    }

    int redirArgC = getRedirArgC(rawArgV, count); //Number of redirections
    char **redirArgV = (char **) malloc((redirArgC + 1) * sizeof(char *));
    if (redirArgC > 0) {
        int kk;
        for (kk = 0; kk <= redirArgC; kk++)
            redirArgV[kk] = rawArgV[kk + cmdArgC]; //Get redirection arguments
    }

    if (rawArgV == NULL) // If line is empty skip line
        return;


#if(DEBUG_MODE == 1) //If Debug mode is on (1), specifiy the below

    printf("Executing command %s with arguments \"", rawArgV[0]);

    if (cmdArgC > 0) {
        int kk;
        for (kk = 1; kk < cmdArgC; kk++) {
            printf("%s", cmdArgV[kk]);
            if (cmdArgC - kk > 1 || redirArgC > 0)
                printf(", ");
        }
    }

    if (redirArgC > 0) {
        int kk;
        for (kk = 0; kk < redirArgC; kk++) {
            printf("%s", redirArgV[kk]);
            if (redirArgC - kk > 1)
                printf(", ");
        }
    }
    printf("\"\n");

#endif

    forkProcess(redirArgC, redirArgV, rawArgV, cmdArgV); //Fork with given parameters

    free(rawArgV); //Free up all malloced things
    free(cmdArgV);
    free(redirArgV);
}

int main(int argc, char **argv) {
    FILE *fp;
    char *line = NULL;
    size_t len = 0;

    //Reads from stdin or from file with kriSh script
    if (argc == 1)
        fp = stdin;
    else fp = fopen(argv[1], "r");
    printf("\nkriSh $ ");

    while (getline(&line, &len, fp) != -1) {
        if (line[0] == '#') //Gets rid of comments
            continue;
        if (!strcmp(line, "exit\n"))
            break;
        execCmd(line); //Executes each command
        printf("\nkriSh $ ");
    }
    free(line);
    exit(0);
}
