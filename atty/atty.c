#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <termios.h>
#include <ctype.h>

#define MAX_LENGTH 2049

int err(const char *format, ...) {
    va_list arg;
    va_start (arg, format);
    vfprintf(stderr, format, arg);
    va_end(arg);
    return EXIT_FAILURE;
}

void usage() {
    err("Usage: ./tcp_send hostName port <input_stream\n");
    exit(EXIT_FAILURE);
}

int isSpace(char c){
    return (c == ' ' || c == '\t' || c == '\0');
}


int lineread() {
    char c;
    char buff[MAX_LENGTH];
    int len = 0;
    char newbuff[5] = "\b \b\0";

    while (read(0, &c, 1) > 0) {
        switch (c) {
            case VERASE:
                if(!len)
                    continue;
                write(1, newbuff, strlen(newbuff));
                buff[--len] = '\0';
                break;
            case VWERASE:
                while (len && isSpace(buff[len])) {
                    buff[len--] = '\0';
                    write(1, newbuff, strlen(newbuff));
                }
                while (len && !isSpace(buff[len])) { //While word
                    buff[len--] = '\0';
                    write(1, newbuff, strlen(newbuff));
                }
                break;
            case VKILL:
                while (len) {
                    buff[len] = '\0';
                    write(1, newbuff, strlen(newbuff));
                    len--;
                }
                buff[len] = '\0';
                write(1, newbuff, strlen(newbuff));
                break;
            case VEOF:
                return 0;
            case '\n':
                buff[len] = '\n';
                len++;
                return 1;
            default:
                buff[len++] = c;
                write(1, &c, 1);
                break;
        }

        if (strlen(buff) == MAX_LENGTH - 1)
            return 1;
    }
    return 0;
}

int main(int argc, char **argv) {
    struct termios term;

    if (tcgetattr(STDIN_FILENO, &term) < 0)
        return err("Error on tcgetattr with code %d: %s\n", errno, strerror(errno));

    term.c_lflag = 0;
    term.c_cc[VMIN] = 1;
    term.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &term) == -1)
        return err("Error on tcsetattr with code %d: %s\n", errno, strerror(errno));

    while (lineread())
        printf("\n");

    return 0;
}
