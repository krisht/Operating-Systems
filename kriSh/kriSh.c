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
    //Set up timers
    struct timeval startTime, endTime;
    struct rusage resUsage;
    int status;

    if (gettimeofday(&startTime, NULL) < 0)
        specialExit("1", "Error getting start time: %s\n", strerror(errno));

    pid_t pid = fork();

    switch (pid) {
        case 0: {
            // Resolve I/O redirection
            int kk, fd, fileNum;
            printf("%s", redirArgV[0]);
            for (kk = 0; kk < redirArgC; ++kk) {
                char * fileName = NULL;
                if (redirArgV[kk][0] == '<') {
                    // Redirect stdin
                    fileName = redirArgV[kk];
                    ++fileName;
                    fd = open(fileName, O_RDONLY);
                    fileNum = STDIN_FILENO;
                } else if (redirArgV[kk][0] == '>' && redirArgV[kk][1] != '>') {
                    // Redirect stdout (Open/Create/Truncate)
                    fileName = redirArgV[kk];
                    ++fileName;
                    fd = open(fileName, O_WRONLY | O_TRUNC | O_CREAT, S_IREAD | S_IWRITE);
                    fileNum = STDOUT_FILENO;
                } else if (redirArgV[kk][0] == '2' && redirArgV[kk][1] == '>') {
                    // Redirect stderr (Open/Create/Truncate)
                    fileName = redirArgV[kk];
                    fileName += 2;
                    fd = open(fileName, O_WRONLY | O_TRUNC | O_CREAT, S_IREAD | S_IWRITE);
                    fileNum = STDERR_FILENO;
                } else if (redirArgV[kk][0] == '>' && redirArgV[kk][1] == '>') {
                    // Redirect stdout (Open/Create/Append)
                    fileName = redirArgV[kk];
                    fileName += 2;
                    fd = open(fileName, O_WRONLY | O_APPEND | O_CREAT, S_IREAD | S_IWRITE);
                    fileNum = STDOUT_FILENO;
                } else if (redirArgV[kk][0] == '2' && redirArgV[kk][1] == '>' && redirArgV[kk][2] == '>') {
                    // Redirect stderr (Open/Create/Append)
                    fileName = redirArgV[kk];
                    fileName += 3;
                    fd = open(fileName, O_WRONLY | O_APPEND | O_CREAT, S_IREAD | S_IWRITE);
                    fileNum = STDERR_FILENO;
                }
                if (fd < 0) {
                    fprintf(stderr, "Can't open file %s: %s\n", fileName, strerror(errno));
                    exit(1);
                }

                // Dup files and close dangling descriptors
                if (dup2(fd, fileNum) < 0) {
                    fprintf(stderr, "Can't dup2 %s to fd %d: %s\n", fileName, fileNum, strerror(errno));
                    exit(1);
                }
                close(fd);
            }

            execvp(rawArgV[0], cmdArgV);
            free(rawArgV);
            break;
        }
        case -1:
            specialExit("1", "Fork failed: %s\n", strerror(errno));
            break;
        default:
            if (wait3(&status, 0, &resUsage) < 0)
                specialExit("1", "Error with wait3: %s\n", strerror(errno));

            if (gettimeofday(&endTime, NULL) < 0)
                specialExit("1", "Error with getting end time: %s\n", strerror(errno));

            int realsecdiff = (int) (endTime.tv_sec - startTime.tv_sec);
            int realmicrodiff = (int) (endTime.tv_usec - startTime.tv_usec);
            printf("Command returned with return code %d,\n", WEXITSTATUS(status));
            printf("consuming %01d.%03d real, %01d.%03d user, %01d.%03d system seconds.\n", realsecdiff, realmicrodiff,
                   (int) resUsage.ru_utime.tv_sec, (int) resUsage.ru_utime.tv_usec, (int) resUsage.ru_stime.tv_sec,
                   (int) resUsage.ru_stime.tv_usec);

    }
}

void execCmd(char *line) {
    int count = 0;
    char **rawArgV = getArgs(line, &count);
    int cmdArgC = getCmdArgC(rawArgV, count);
    char **cmdArgV = (char **) malloc((cmdArgC + 1) * sizeof(char *));

    if (cmdArgC > 0) {
        int kk;
        for (kk = 0; kk <= cmdArgC; kk++)
            cmdArgV[kk] = rawArgV[kk]; // Add arg to cmdArgV

        cmdArgV[cmdArgC] = 0; //Add null terminator
    }

    int redirArgC = getRedirArgC(rawArgV, count);
    char **redirArgV = (char **) malloc((redirArgC + 1) * sizeof(char *));
    if (redirArgC > 0) {
        int kk;
        for (kk = 0; kk <= redirArgC; kk++)
            redirArgV[kk] = rawArgV[kk + cmdArgC];

        //Add another arg to redirArgV but offset by cmdArgC to jump past cmd args'
    }

    //Display info about command exec

    if (rawArgV == NULL)
        return;

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

    forkProcess(redirArgC, redirArgV, rawArgV, cmdArgV);

    free(rawArgV);
    free(cmdArgV);
    free(redirArgV);
    return;
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
        if (line[0] == '#') //gets rid of comments
            continue;
        if (!strcmp(line, "exit\n"))
            break;
        execCmd(line); //executes each command
        printf("\nkriSh $ ");
    }
    free(line);
    exit(0);
}