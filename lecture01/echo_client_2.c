#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h> // 통신을 위한 header
#include <sys/socket.h> // 통신을 위한 header

#define BUF_SIZE 1024

void error_handling(char *message);

int main(int argc, char *argv[])
{
	int sock;
	char message[BUF_SIZE];
	int str_len, recv_len, recv_cnt;
	struct sockaddr_in serv_adr; //arpa/inet.h 에 정의되어 있음

	if(argc!=3)
	{
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
	//sys/socket.h 에 정의
	sock = socket(PF_INET, SOCK_STREAM, 0); //socket을 정의
	if(sock == -1)
	{
		error_handling("socket() ERROR");
	}

	memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET; // 통신 방식 정의
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]); // IP 주소 할당
    serv_adr.sin_port = htons(atoi(argv[2])); //포트 주소 할당

    if(connect(sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
    {
        error_handling("connect() error");
    }
    else
    {
        puts("connect...");
    }

    while(1)
    {
        fputs("Input message(Q, q to end):", stdout);
        fgets(message, BUF_SIZE, stdin);

        if(!strcmp(message, "q\n") || !strcmp(message, "Q\n")) {
            break;
        }

        str_len = write(sock, message, strlen(message));
        recv_len = 0;
        while (recv_len < str_len) {
            recv_cnt = read(sock, &message[recv_len], BUF_SIZE - 1);
            if (recv_cnt == -1) {
                error_handling("read() error");
            }
            recv_len += recv_cnt;
        }
        message[recv_len] = 0;
//        write(sock, message, strlen(message));

//        str_len = read(sock, message, BUF_SIZE-1);
//        message[str_len] = 0;

        printf("message from server %s \n", message);
    }

    close(sock);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}





