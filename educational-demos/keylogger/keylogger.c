/*

use the keyboard event file in /dev/
take argument
we cannot use fopen or fread to read events because of blocking behavior
use sys functions provided by kernel to read keyboard events (documentation for system functions are found in man 2), check man syscalls
open() returns int file descriptor

*/

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Eventfile argument required");
        return 1;
    }

    int fd = open(argv[1], O_RDONLY, 0);


    close(fd);
}
