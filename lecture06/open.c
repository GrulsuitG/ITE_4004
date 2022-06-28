#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

int main(){
    int fd, n, ret;

    char buf[128];
    struct timeval tv;

    fd_set readfs;
    fd = open("write.txt", O_RDONLY);

    if (fd < 0) {
        perror("file open error");
        exit(1);
    }

    memset(buf, 0x00, 128);

    FD_ZERO(&readfs);
    while (1) {
        FD_SET(fd, &readfs);
        ret = select(fd + 1, &readfs, NULL, NULL, NULL);

        if (ret == -1) {
            perror("select() error");
            exit(1);
        }
        if (FD_ISSET(fd, &readfs)) {
            while ((n = read(fd, buf, 128)) > 0) {
                printf("%s", buf);
            }
        }

        memset(buf, 0x00, 128);
        usleep(1000);
    }
}