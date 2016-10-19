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

void specialExit(const char *retVal, const char *format, ...) {
    va_list arg;
    va_start (arg, format);
    vfprintf(stderr, format, arg);
    va_end(arg);
    if (!strcmp(retVal, "NULL"))
        return;
    else exit(atoi(retVal));

}

char **getArgs(char *line, int *count) {
    char **args = NULL;
    char *delimiters = "\t ";
    line[strlen(line) - 1] = 0;

    char *arg = strtok(line, delimiters);

    while (arg != NULL) {
        args = realloc(args, ++(*count) * sizeof(char *));
        if (args == NULL)
            specialExit("1", "Error reallocating args memory.\n");

        args[*count - 1] = arg;
        arg = strtok(NULL, delimiters);
    }
    return args;
}

// Returns amount of arguments excluding redirections
int getCmdArgC(char **args, int count) {
    int i;
    int cmdArgC = 0;
    for (i = 0; i < count; ++i) {
        char *a = args[i];
        if (!(a[0] == '<' || a[0] == '>' || (a[0] == '2' && a[1] == '>')))
            cmdArgC++;
    }
    return cmdArgC;
}

// Returns amount of redirection arguments
int getRedirArgC(char **args, int count) {
    int i;
    int redirArgC = 0;
    for (i = 0; i < count; ++i) {
        char *a = args[i];
        if ((a[0] == '<' || a[0] == '>' || (a[0] == '2' && a[1] == '>')))
            redirArgC++;
    }
    return redirArgC;
}

int forkProcess(int redirArgC, char **redirArgV, char **rawArgV, char **cmdArgV) {


    struct timeval startTime, endTime;
    if (gettimeofday(&startTime, NULL) < 0)
        specialExit("1", "Error getting start time: %s\n", strerror(errno));

    pid_t pid;
    struct rusage ru;
    int status;

    switch (pid = fork()) {
        case 0: {
            int redi, fd, fileNum;
            for (redi = 0; redi < redirArgC; ++redi) {
                char *fileName = NULL;
                if (redirArgV[redi][0] == '<') {
                    // Redirect stdin
                    fileName = redirArgV[redi];
                    ++fileName;
                    fd = open(fileName, O_RDONLY);
                    fileNum = STDIN_FILENO;
                } else if (redirArgV[redi][0] == '>' && redirArgV[redi][1] != '>') {
                    // Redirect stdout (Open/Create/Truncate)
                    fileName = redirArgV[redi];
                    ++fileName;
                    fd = open(fileName, O_WRONLY | O_TRUNC | O_CREAT, S_IREAD | S_IWRITE);
                    fileNum = STDOUT_FILENO;
                } else if (redirArgV[redi][0] == '2' && redirArgV[redi][1] == '>') {
                    // Redirect stderr (Open/Create/Truncate)
                    fileName = redirArgV[redi];
                    fileName += 2;
                    fd = open(fileName, O_WRONLY | O_TRUNC | O_CREAT, S_IREAD | S_IWRITE);
                    fileNum = STDERR_FILENO;
                } else if (redirArgV[redi][0] == '>' && redirArgV[redi][1] == '>') {
                    // Redirect stdout (Open/Create/Append)
                    fileName = redirArgV[redi];
                    fileName += 2;
                    fd = open(fileName, O_WRONLY | O_APPEND | O_CREAT, S_IREAD | S_IWRITE);
                    fileNum = STDOUT_FILENO;
                } else if (redirArgV[redi][0] == '2' && redirArgV[redi][1] == '>' && redirArgV[redi][2] == '>') {
                    // Redirect stderr (Open/Create/Append)
                    fileName = redirArgV[redi];
                    fileName += 3;
                    fd = open(fileName, O_WRONLY | O_APPEND | O_CREAT, S_IREAD | S_IWRITE);
                    fileNum = STDERR_FILENO;
                }
                if (fd < 0)
                    specialExit("1", "Can't open file %s: %s\n", fileName, strerror(errno));

                // Dup files and close dangling descriptors
                if (dup2(fd, fileNum) < 0)
                    specialExit("1", "Can't dup2 %s to fd %d: %s\n", fileName, fileNum, strerror(errno));
                close(fd);
            }

            execvp(rawArgV[0], cmdArgV);
            free(rawArgV);
            break;

        }
        case -1:
            specialExit("1""Fork failed: %s\n", strerror(errno));
            break;
        default:
            if (wait3(&status, 0, &ru) < 0)
                specialExit("1", "Wait3 error: %s\n", strerror(errno));

            if (gettimeofday(&endTime, NULL) < 0)
                specialExit("1", "Error getting end time: %s\n", strerror(errno));

            int realsecdiff = endTime.tv_sec - startTime.tv_sec;
            int realmicrodiff = endTime.tv_usec - startTime.tv_usec;
            printf("Command returned with return code %d,\n", WEXITSTATUS(status));
            printf("consuming %01d.%03d real seconds, %01d.%03d user, %01d.%03d system.\n",
                   realsecdiff, realmicrodiff, ru.ru_utime.tv_sec, ru.ru_utime.tv_usec, ru.ru_stime.tv_sec,
                   ru.ru_stime.tv_usec);

    }
}

int execCmd(char *line, char **argv, int argc) {
    int count = 0;

    char **rawArgV = getArgs(line, &count);
    int cmdArgC = getCmdArgC(rawArgV, count);

    char **cmdArgV = malloc(cmdArgC * sizeof(char *));
    if (cmdArgC > 0) {
        int kk;
        for (kk = 0; kk <= cmdArgC; ++kk)
            cmdArgV[kk] = rawArgV[kk]; // Add add arg to commandArgs

        cmdArgV = realloc(cmdArgV, (cmdArgC + 1) * sizeof(char *));
        cmdArgV[cmdArgC] = 0; //Add null terminator
    }

    int redirArgC = getRedirArgC(rawArgV, count);
    char **redirArgV = malloc(redirArgC * sizeof(char *));
    if (redirArgC > 0) {
        int kk;
        for (kk = 0; kk <= redirArgC; kk++)
            redirArgV[kk] = rawArgV[kk +
                                    cmdArgC]; //Add another arg to redirArgV but offset by cmdArgC to jump past cmd args
    }

    //Display info about command exec

    printf("Executing command %s with arguments\"", rawArgV[0]);

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
    return 0;
}

int main(int argc, char **argv) {
    FILE *fp;
    char *line = NULL;
    size_t len = 0;

    if (argc == 1)
        fp = stdin;
    else fp = fopen(argv[1], "r");

    while (getline(&line, &len, fp) != -1) {
        if (line[0] == '#')
            continue;
        execCmd(line, argv, argc);
    }

    free(line);
    exit(0);
}