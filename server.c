#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUF_SIZE 100
#define MAX_CLNT 256

#define TRUE 1
#define FALSE 0

void* handle_clnt(void* arg);
void* game_start(void* arg);
void send_msg(char *msg, int len, int sender);
void error_handling(char *msg);

int clnt_cnt = 0;
int wait_player = 0;
int clnt_socks[MAX_CLNT];
int first = TRUE;
int wait_socks[2] = {0, 0};
pthread_mutex_t mutx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int main(int argc, char* argv[]){
    int serv_sock, clnt_sock, opt;
    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t clnt_adr_sz;
    pthread_t t_id, t_id2;
    if (argc != 2) {
        printf("usage: %s <port>\n", argv[0]);
        exit(1);
    }

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);

    opt = 1;
    setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY); //INADDR_ANY 시스템에 ip 주소를 자동으로 할당
    serv_adr.sin_port = htons(atoi(argv[1]));

    if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr)) == -1){
        error_handling("bind() error");
    }
    if(listen(serv_sock, 5) == -1) {
        error_handling("listen() error");
    }

    pthread_create(&t_id2, NULL, game_start, (void*)NULL);
    pthread_detach(t_id2);
    while(1){
        clnt_adr_sz = sizeof(clnt_adr);
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);

        pthread_mutex_lock(&mutx);
        clnt_socks[clnt_cnt++] = clnt_sock;
        wait_player++;
        printf("wait player : %d\n", wait_player);
        if (wait_player == 2){
            pthread_cond_signal(&cond);
        }
        pthread_mutex_unlock(&mutx);

        pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
        pthread_detach(t_id);
        printf("connected clinet IP: %s \n", inet_ntoa(clnt_adr.sin_addr));

    }
    close(serv_sock);
    return 0;
}

void* handle_clnt(void *arg){
    int clnt_sock = *((int*)arg);
    int str_len = 0, i;
    char msg[BUF_SIZE];

    pthread_mutex_lock(&mutx);
    if(first){
        write(clnt_sock, "F", 2);
        first = FALSE;
    } else{
        first = TRUE;
    }
    pthread_mutex_unlock(&mutx);

    while((str_len = read(clnt_sock, msg, sizeof(msg))) != 0){
        send_msg(msg,str_len, clnt_sock);
    }

    pthread_mutex_lock(&mutx);
    for (i = 0; i < clnt_cnt; i++) {
        if (clnt_sock == clnt_socks[i]) {
            while(i++<clnt_cnt-1) {
                clnt_socks[i] = clnt_socks[i+1];
            }
            break;
        }
    }
    if (wait_player) wait_player--;
    clnt_cnt--;
    pthread_mutex_unlock(&mutx);
    close(clnt_sock);
    return NULL;
}

void* game_start(void* arg){
    char msg[BUF_SIZE];
    while(1){
        if(wait_player == 2){
            sprintf(msg, "S");
            send_msg("S", 2, -1);
            wait_player = 0;
            for (int i = 0; i < 2; ++i) {
                wait_socks[i] = 0;
            }
        }
        else{
            pthread_mutex_lock(&mutx);
            pthread_cond_wait(&cond, &mutx);
            pthread_mutex_unlock(&mutx);
        }
    }
    return NULL;
}

void send_msg(char *msg, int len, int sender){
    int i;
    pthread_mutex_lock(&mutx);
    for (i = 0; i < clnt_cnt; i++) {
        if (clnt_socks[i] != sender) {
            write(clnt_socks[i], msg, len);
        }
    }
//    fputs(msg, stdout);
//    printf("\n");
    pthread_mutex_unlock(&mutx);
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}







