#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <termios.h>

#define BUFF_SIZE 2049

struct termios saved;
char buff[BUFF_SIZE];

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
    return (c == ' ' || c == '\t'); //|| c == '\n' || c == '\v' || c == '\r' || c=='\f'); //screw these for now
}

void resetInputMode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &saved) == -1)
        exit(err("Error on resetting to original (saved) mode with code %d: %s.\n", errno, strerror(errno)));
}

void setInputMode() {
    struct termios tattr;

    if (!isatty(STDIN_FILENO))
        exit(err("Error on using stdin as tty with code %d: %s.\n", errno, strerror(errno)));

    if (tcgetattr(STDIN_FILENO, &saved) == -1)
        exit(err("Error on tcgetattr to save original settings with code %d: %s.\n", errno, strerror(errno)));
    
    atexit(resetInputMode);

    tcgetattr(STDIN_FILENO, &tattr);
    tattr.c_lflag &= ~(ICANON | ECHO); //Removes echo and icanon
    tattr.c_cc[VMIN] = 1;
    tattr.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &tattr) == -1)
        exit(err("Error on tcsetattr to set to terminal to non-canonical mode with code %d: %s.\n", errno,
                 strerror(errno)));
}

int myread() {
    char c;
    int len = 0, count = BUFF_SIZE - 1; // Set count (num characters to read) to be maximum

    memset(buff, '\0', BUFF_SIZE); // Initialize the buffer to nulls

    char deleteOne[3] = "\b \b"; //"Deleting" on the tty

    while (read(STDIN_FILENO, &c, 1) > 0) { //Read EOF condition
        switch (c) {
            case CERASE: { //Erase one character received backspace
                if (!len)
                    continue;
                write(STDOUT_FILENO, deleteOne, strlen(deleteOne));
                buff[--len] = '\0';
                break;
            }
            case CWERASE: {
            	// Delete spaces before word
                while (len && (isSpace(buff[len]) || buff[len] == '\0')) {
                    buff[len--] = '\0';
                    write(STDOUT_FILENO, deleteOne, strlen(deleteOne));
                }
                //Delete word itself
                while (len && (!isSpace(buff[len]) || buff[len] == '\0')) {
                    buff[len--] = '\0';
                    write(STDOUT_FILENO, deleteOne, strlen(deleteOne));
                }
                buff[len] = '\0'; //Delete extra character not deleted in loop
                break;
            }
            case CKILL: { //Delete entire line
                while (len) {
                    buff[len--] = '\0';
                    write(STDOUT_FILENO, deleteOne, strlen(deleteOne));
                }
                buff[len] = '\0'; //Delete extra character not deleted in loop
                //write(STDOUT_FILENO, deleteOne, strlen(deleteOne));
                break;
            }
            case CEOF: {
                return len;
            }
            case '\n': {
                buff[len++] = '\n';
                return len;
            }
            default: {
                buff[len++] = c;
                write(STDOUT_FILENO, &c, 1);
            }
        }
        if (strlen(buff) == count) //Buffer overloaded
            break; 
    }
    return len;
}

int main(int argc, char **argv) {
    int readlen = 0;
    setInputMode();
    while ((readlen = myread())){
        fprintf(stderr, "\nLength: %d : '%s'\n", readlen, buff);
    }
    return 0;
}