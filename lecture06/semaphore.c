#include <stdio.h>
#include <pthread.h>
#include <sys/semaphore.h>
#include <dispatch/dispatch.h>
#include <errno.h>
#include <stdlib.h>

void *reada(void *arg);
void *accua(void *arg);
//static sem_t sem_one;
//static sem_t sem_two;
static dispatch_semaphore_t sem_one;
static dispatch_semaphore_t sem_two;
static int num;

int main(int argc, char *argv[]){
    pthread_t id_t1, id_t2;

    sem_one = dispatch_semaphore_create(0);
    sem_two = dispatch_semaphore_create(1);

//    int ret1 = sem_init(&sem_one, 0, 0);
//    int ret2 = sem_init(&sem_two, 0, 1);
    pthread_create(&id_t1, NULL, reada, NULL);
    pthread_create(&id_t2, NULL, accua, NULL);

    pthread_join(id_t1, NULL);
    pthread_join(id_t2, NULL);

//    sem_destroy(&sem_one);
//    sem_destroy(&sem_two);
    return 0;
}

void * reada(void *arg){
    int i;
    for (i = 0; i < 5; i++) {
//        sem_wait(&sem_two);
        dispatch_semaphore_wait(sem_two, DISPATCH_TIME_FOREVER);
        fputs("input num:", stdout);
        scanf("%d", &num);
        dispatch_semaphore_signal(sem_one);
//        sem_post(&sem_one);
    }
    return NULL;
}

void * accua(void *arg){
    int sum = 0, i;
    for (i = 0; i < 5; i++) {
        dispatch_semaphore_wait(sem_one, DISPATCH_TIME_FOREVER);
//        sem_wait(&sem_one);
        sum += num;
//        sem_post(&sem_two);
        dispatch_semaphore_signal(sem_two);

    }

    printf("result: %d \n", sum);
    return NULL;
}



