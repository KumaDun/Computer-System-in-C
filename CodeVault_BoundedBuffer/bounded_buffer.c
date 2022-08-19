/*
Bounded_Buffer is to
1. Manage shared memory access
2. Checking for buffer is full
3. Checking for buffer is empty 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>

#define THREAD_NUM 8

int buffer[10];
int count = 0;

sem_t semEmpty; //How many empty slots you have
sem_t semFull; //How many assigned slots you have
pthread_mutex_t mutexBuffer;

void * producer (void * args) {
    while (1) {
        //Produce 
        int x = rand() %100;
        //Add to the buffer
        sem_wait(&semEmpty);
        pthread_mutex_lock(&mutexBuffer);
        buffer[count] = x;
        count ++;
        pthread_mutex_unlock(&mutexBuffer);
        sem_post(&semFull);
    }
}

void * consumer (void * args) {
    while (1) {
        int y = -1;
        //Revmoes from buffer

        sem_wait(&semFull);
        pthread_mutex_lock(&mutexBuffer);
        y = buffer[count - 1];
        count --;
        //Consume
        printf("Got %d \n", y);
        pthread_mutex_unlock(&mutexBuffer);
        sem_post(&semEmpty);
        sleep(1);
    }
}

int main(int arc, char* argv[]) {
    srand(time(NULL));
    pthread_t th [THREAD_NUM];
    pthread_mutex_init(&mutexBuffer, NULL);

    sem_init(&semEmpty,0,10);
    sem_init(&semFull,0,0);

    int i;
    for (i=0; i<THREAD_NUM; i++) {
        if (i % 2 == 0) {
            if (pthread_create(&th[i], NULL, &producer, NULL) != 0) {
                perror ("Failed to create thread");
            }
        }else {
            if (pthread_create(&th[i],NULL, &consumer, NULL) !=0 ) {
                perror ("Failed to create thread");
            }
        }
    }

    for (i=0; i<THREAD_NUM; i++) {
        if (pthread_join(th[i], NULL) != 0){
            perror("Failed to join thread");
        }
    }
    sem_destroy(&semEmpty);
    sem_destroy(&semFull);
    pthread_mutex_destroy(&mutexBuffer);
    return 0;
}