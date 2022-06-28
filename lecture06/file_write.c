#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]){
    int fd, ret;
    fd = open("write.txt", O_CREAT | O_WRONLY | O_APPEND);

    if (fd < 0) {
        perror("file error");
        exit(1);
    }

    if (argc != 2) {
        printf("usage: ./%s <message>\n", argv[0]);
        exit(1);
    }

    strcat(argv[1], "\n");
    ret = write(fd, argv[1], strlen(argv[1]));
    if (ret < 0) {
        perror("write error");
        exit(1);
    }

    close(fd);
    return 0;
}
