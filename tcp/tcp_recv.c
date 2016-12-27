#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/time.h>

#define BUFF_SIZE 4096

int err(const char *format, ...) {
    va_list arg;
    va_start (arg, format);
    vfprintf(stderr, format, arg);
    va_end(arg);
    return EXIT_FAILURE;
}

void usage() {
    err("Usage: ./tcp_rest port >output_file\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {

    if (argc != 3)
        usage();

    struct hostent *hostEntry;
    struct sockaddr_in sockIn;
    struct timeval start, end;
    char buff[BUFF_SIZE], *writeBuff;
    unsigned short port;
    int sock1, sock2, rBytes, wBytes, numBytes = 0, len;

    double start_time, end_time, rate;

    port = (unsigned short) atoi(argv[1]);

    sockIn.sin_family = AF_INET;
    sockIn.sin_port = htons(port);
    sockIn.sin_addr.s_addr = INADDR_ANY;


    if ((sock1 = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return err("Error on opening socket with code %d: %s\n", errno, strerror(errno));

    if (bind(sock1, (struct sockaddr *) &sockIn, sizeof(sockIn)) < 0) {
        close(sock1);
        return err("Error on binding socket with code %d: %s\n", errno, strerror(errno));
    }

    if (listen(sock1, 128) < 0)
        return err("Error on listening to socket 1 with code %d: %s\n", errno, strerror(errno));

    err("Listening to socket for a connection!");

    if ((sock2 = accept(sock1, (struct sockaddr *) &sockIn, (unsigned int *) &len)) < 0)
        return err("Error on accepting connection to socket with code %d: %s\n", errno, strerror(errno));

    err("Accepted socket connection....");

    gettimeofday(&start, NULL);

    while ((rBytes = (int) read(sock2, buff, BUFF_SIZE)) != 0) {
        if (rBytes < 0)
            return err("Error on reading from socket with code %d: %s\n", errno, strerror(errno));
        writeBuff = buff;

        for (wBytes = 0; wBytes < rBytes;)
            if ((wBytes = (int) write(STDOUT_FILENO, writeBuff, (unsigned int) rBytes)) <= 0)
                err("Error on writing to stdout with code %d: %s\n", errno, strerror(errno));
    }

    if (close(sock2) < 0)
        return err("Error when closing socket with code %d: %s\n", errno, strerror(errno));

    gettimeofday(&end, NULL);
    end_time = end.tv_sec + (double) end.tv_usec / 1000000;
    start_time = start.tv_sec + (double) start.tv_usec / 1000000;
    rate = numBytes / ((end_time - start_time) * 1024 * 1024);
    err("\n\nRemote Endpoint Info: \n\tIP Address: %s\n\tPort: %d\n", inet_ntoa(sockIn.sin_addr),
        ntohs(sockIn.sin_port));

    if ((hostEntry = gethostbyaddr(&sockIn.sin_addr, sizeof(sockIn.sin_addr), AF_INET)))
        err("\tHostname: %s\n", hostEntry->h_name);
    else err("\tHostname: unknown\n");

    err("\n\nReceived %d bytes at a rate of %.6f MB/s\n", numBytes, rate);

    if (close(sock1) < 0)
        err("Error on closing socket with code %d: %s\n", errno, strerror(errno));

    return EXIT_SUCCESS;
}