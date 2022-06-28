#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <ctype.h>
#include <time.h>

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_RESET "\x1b[0m"

#define BUF_SIZE 100
#define NUM_SIZE 20
#define BOARD_SIZE 5
#define BINGO_TARGET 3
#define MAX_NUM 50

#define TRUE 1
#define FALSE 0

#define MY_TURN 1
#define OTHER_TURN 0
#define END 2


void* send_msg(void* arg);
void* recv_msg(void* arg);
void error_handling(char *msg);
void game_start();
void make_board();
void print_board();
int inBoard(int num);
int bingo_check();


int board[BOARD_SIZE][BOARD_SIZE];
int check[BOARD_SIZE][BOARD_SIZE] = {FALSE,};
int turn = OTHER_TURN;

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char* argv[]){
    int sock;
    struct sockaddr_in serv_addr;
    pthread_t snd_thread, rcv_thread;
    void *thread_return;

    if (argc != 3) {
        printf("usage: %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; // 통신 방식 정의
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]); // IP 주소 할당
    serv_addr.sin_port = htons(atoi(argv[2])); //포트 주소 할당

    if (connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1) {
        error_handling("connect() error");
    }
    make_board();

    pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);
    pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);
    pthread_join(snd_thread, &thread_return);
    pthread_join(rcv_thread, &thread_return);

    close(sock);

    return 0;

}

void* send_msg(void* arg){
    int sock = *((int*)arg);
    char msg[BUF_SIZE];
    char input[NUM_SIZE];
    int str_len, num, flag;

    pthread_mutex_lock(&mutex);
    fputs("상대방을 기다리고 있습니다.\n", stdout);
    pthread_cond_wait(&cond, &mutex);
    pthread_mutex_unlock(&mutex);

    while(1){
        print_board();
        memset(&msg, 0, BUF_SIZE);
        memset(&input, 0, NUM_SIZE);
        if(turn == MY_TURN) {
            fputs("숫자를 입력해주세요:", stdout);
            fgets(input, NUM_SIZE, stdin);
            // 종료 처리
            if (input[0] == 'q' || input[0] == 'Q'){
                fputs("게임을 종료했습니다.\n", stdout);
                write(sock, "Q", 2);
                close(sock);
                exit(0);
            }
            // 숫자 입력 처리
            if((num = atoi(input)) != 0){
                // 보드에 없는 숫자를 입력한 경우
                if (!inBoard(num)) {
                    fputs("빙고판에 있는 숫자를 입력해주세요.\n", stdout);
                    continue;
                }
                if (bingo_check() == BINGO_TARGET) {
                    sprintf(msg, "W%d", num);
                    write(sock, msg, strlen(msg));
                    pthread_exit(0);
                } else {
                    sprintf(msg, "N%d", num);
                    write(sock, msg, strlen(msg));
                }
                turn = OTHER_TURN;
            }
        } else if(turn == OTHER_TURN){
            pthread_mutex_lock(&mutex);
            fputs("상대방의 차례입니다.\n", stdout);
            pthread_cond_wait(&cond, &mutex);
            turn = MY_TURN;
            pthread_mutex_unlock(&mutex);
        }

    }
    return NULL;
}
void* recv_msg(void* arg){
    int sock = *((int*)arg);
    char read_msg[BUF_SIZE];
    int str_len, num;

    while(1) {
        memset(&read_msg, 0, BUF_SIZE);
        str_len = read(sock, read_msg, BUF_SIZE - 1);
        if (read_msg[0] == 'Q' || read_msg[0] == 'D' || read_msg[0] == 'L') {
            print_board();
            if (read_msg[0] == 'Q'){
                fputs("상대방이 게임을 떠났습니다.\n", stdout);
            }
            if (read_msg[0] == 'D') {
                fputs("비겼습니다!\n", stdout);
            } else if (read_msg[0] == 'L'){
                fputs("당신이 이겼습니다!\n", stdout);
            }
            close(sock);
            exit(0);
        }
        else if(read_msg[0] == 'S'){
            pthread_mutex_lock(&mutex);
            pthread_cond_signal(&cond);
            pthread_mutex_unlock(&mutex);
        }
        else if(read_msg[0] == 'F'){
            turn = MY_TURN;
        }
        else if(read_msg[0] == 'N' || read_msg[0] == 'W'){
            char read_num[NUM_SIZE];
            memset(&read_num, 0, NUM_SIZE);
            for (int i = 1; i < str_len; i++) {
                read_num[i - 1] = read_msg[i];
            }
            fputs("상대방이 ", stdout);
            fputs(read_num, stdout);
            fputs("를 입력했습니다.\n", stdout);
            num = atoi(read_num);

            inBoard(num);
            if (read_msg[0] == 'W') {
                print_board();
                if (bingo_check() == BINGO_TARGET) {
                    write(sock, "D", 2);
                    fputs("비겼습니다!\n", stdout);
                } else {
                    write(sock, "L", 2);
                    fputs("당신은 졌습니다!\n", stdout);
                }
                close(sock);
                exit(0);
            }
            else if (read_msg[0] == 'N') {
                if (bingo_check() == BINGO_TARGET) {
                    write(sock, "W", 2);
                }
                pthread_mutex_lock(&mutex);
                pthread_cond_signal(&cond);
                pthread_mutex_unlock(&mutex);
            }
        }

    }
    return NULL;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

void make_board(){
    int check_num[MAX_NUM + 1] = {0,};
    int num_arr[BOARD_SIZE * BOARD_SIZE];
    int cnt = 0;
    srand(time(NULL));
    while (cnt < BOARD_SIZE * BOARD_SIZE) {
        int num = rand() % MAX_NUM + 1;
        if (check_num[num]) {
            continue;
        }
        num_arr[cnt++] = num;
        check_num[num] = 1;
    }

    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            board[i][j] = num_arr[i*BOARD_SIZE + j];
//            board[i][j] = i*BOARD_SIZE + j + 1;
        }
    }
}

void print_board(){
    printf("현재 빙고 줄 : %d\n", bingo_check());
    printf("+----+----+----+----+----+\n");
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            if(check[i][j] == 1){
                printf(ANSI_COLOR_RESET "|");
                printf(ANSI_COLOR_RED " %2d ", board[i][j]);
            }
            else
                printf(ANSI_COLOR_RESET "| %2d ", board[i][j]);
        }
        printf(ANSI_COLOR_RESET "|\n");
        printf("+----+----+----+----+----+\n");
    }
}

int bingo_check(){
    int count=0;

    for(int i=0; i < BOARD_SIZE; i++)
    {
        if(check[i][0] & check[i][1] & check[i][2] & check[i][3] & check[i][4])
            count++;
        if(check[0][i] & check[1][i] & check[2][i] & check[3][i] & check[4][i])
            count++;
    }
    if(check[0][0] & check[1][1] & check[2][2] & check[3][3] & check[4][4])
        count++;
    if(check[0][4] & check[1][3] & check[2][2] & check[3][1] & check[4][0])
        count++;
    return count;
}

int inBoard(int num){
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            if(board[i][j] == num){
                check[i][j] = TRUE;
                return TRUE;
            }
        }
    }
    return FALSE;
}