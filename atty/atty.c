#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <termios.h>

#define BUFF_SIZE 129

struct termios saved;

int err(const char *format, ...) {
    va_list arg;
    va_start (arg, format);
    vfprintf(stderr, format, arg);
    va_end(arg);
    return EXIT_FAILURE;
}

void usage() {
    err("Usage: ./atty\n");
    exit(EXIT_FAILURE);
}

int isSpace(char c) {
    return (c == ' ' || c == '\t'); //|| c == '\n' || c == '\v' || c == '\r' || c=='\f');
}

void resetInputMode() {
    if (tcsetattr(STDIN_FILENO, TCSANOW, &saved) == -1)
        exit(err("Error on resetting to original (saved) mode with code %d: %s\n", errno, strerror(errno)));
}

void setInputMode() {
    struct termios tattr;

    if (!isatty(STDIN_FILENO))
        exit(err("Error on using STDIN as tty with code %d: %s\n", errno, strerror(errno)));

    if (tcgetattr(STDIN_FILENO, &saved) == -1)
        exit(err("Error on tcgetattr to save original settings with code %d: %s\n", errno, strerror(errno)));
    atexit(resetInputMode);

    tcgetattr(STDIN_FILENO, &tattr);
    tattr.c_lflag &= ~(ICANON | ECHO); //Removes echo and icanon
    tattr.c_cc[VMIN] = 1;
    tattr.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &tattr) == -1)
        exit(err("Error on tcsetattr to set to terminal to non-canonical mode with code %d: %s\n", errno,
                 strerror(errno)));
}

int myread(char *buff, int count) {
    if (count > BUFF_SIZE - 1)
        count = BUFF_SIZE - 1;
    else if (count < 0)
        count = 1;

    char c;
    int len = 0;
    memset(buff, '\0', BUFF_SIZE);

    char deleteOne[4] = "\b \b\0"; //Doesn't delete properly w/o null term

    while (read(STDIN_FILENO, &c, 1) > 0) {
        switch (c) {
            case CERASE: {
                if (!len)
                    continue;
                write(STDOUT_FILENO, deleteOne, strlen(deleteOne));
                buff[len--] = '\0'; //--len
                break;
            }
            case CWERASE: {
                while (len && (isSpace(buff[len]) || buff[len] == '\0')) {
                    buff[len--] = '\0';
                    write(STDOUT_FILENO, deleteOne, strlen(deleteOne));
                }
                while (len && (!isSpace(buff[len]) || buff[len] == '\0')) {
                    buff[len--] = '\0';
                    write(STDOUT_FILENO, deleteOne, strlen(deleteOne));
                }
                break;
            }
            case CKILL: {
                while (len) {
                    buff[len] = '\0';
                    write(STDOUT_FILENO, deleteOne, strlen(deleteOne));
                    len--;
                }
                buff[len] = '\0';
                write(STDOUT_FILENO, deleteOne, strlen(deleteOne));
                break;
            }
            case CEOF: {
                return len;
            }
            case '\n': {
                //buff[len++] = '\n'; length  count doesn't make sense to me
                return len;
            }
            default: {
                buff[len++] = c;
                write(STDOUT_FILENO, &c, 1);
            }
        }
        if (strlen(buff) == count)
            return len;
    }
    return len;
}


int main(int argc, char **argv) {
    char buff[BUFF_SIZE];
    int count = BUFF_SIZE - 1; //set to max
    int readlen = 0;
    setInputMode();
    while ((readlen = myread(buff, count))) {
        printf("\n");
        fprintf(stderr, "Length: %d : '%s'\n", readlen, buff);
    }
    return 0;
}