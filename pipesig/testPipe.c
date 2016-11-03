#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

void exitWithError(const char *format, ...) {
    va_list arg;
    va_start (arg, format);
    vfprintf(stderr, format, arg);
    va_end(arg);
    exit(EXIT_FAILURE);
}

int main (int argc, char** argv){
	int pfd[2], bytes = 0; 

	if ((pfd[0] = open ("readSide", O_CREAT | O_TRUNC | O_RDONLY, 0666)) < 0)
		exitWithError("ERROR cannot open() readSide: %s", strerror(errno));

	if ((pfd[1] = open ("writeSide", O_CREAT | O_TRUNC | O_WRONLY, 0666)) < 0)
		exitWithError ("ERROR cannot open() writeSide: %s", strerror(errno));

	if (pipe2 (pfd, O_NONBLOCK) < 0)
		exitWithError ("ERROR cannot pipe2() pfd: %s", strerror(errno));

	while (write (pfd[1], "A", 1) > 0) bytes++;

	if (errno == EAGAIN)
		printf ("Size of pipe: %d\n", bytes);
	else exitWithError ("ERROR write() on the writeSide failed: %s", strerror(errno));

	return 0; 

}