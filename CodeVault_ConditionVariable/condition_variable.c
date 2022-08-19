/*
pthread_cond can do 3 things
pthread_cond_wait
pthread_cond_signal
pthread_cond_broadcast
*/


#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#define THREAD_NUM 2

pthread_mutex_t mutexFuel;
pthread_cond_t condFuel;

int fuel = 0;

void * fuel_filling(void* arg) {
    for (int i=0; i<5; i++) {
        pthread_mutex_lock (&mutexFuel);
        fuel += 15;
        printf("Filling fuel ... %d \n", fuel);
        pthread_mutex_unlock(&mutexFuel);
        //pthread_cond_signal does not do anything to mutex, only to condFuel
        //pthread_cond_wait control mutex
        pthread_cond_signal(&condFuel);

        //sleep for 1 second to let car run and check again
        //if sleep removed, will filling the fuel for 5 times before next fuel check in car thread
        sleep(1);
    }
    printf("Filling fuel \n");
}

void * car(void * arg) {
    pthread_mutex_lock(&mutexFuel);

    /*
    It's important to have a while loop to check again.
    pthread_cond_wait is going to unlock mutexFuel and wait on condFuel
    */
    while (fuel < 40) {
        printf("No fuel Waiting : %d \n", fuel);
        //pthread_cond_wait unlock mutexFuel and wait on condFuel
        //Equivalent to:
        //pthread_mutex_unlock(&mutexFuel);
        //wait for signal on condFuel
        pthread_cond_wait(&condFuel, &mutexFuel);
    }

    fuel -= 40;
    printf("Got fuel. Now left: %d \n", fuel);
    pthread_mutex_unlock(&mutexFuel);
}


int main(int argc, char* argv[]) {
    pthread_t th[THREAD_NUM];
    pthread_mutex_init(&mutexFuel, NULL);
    pthread_cond_init(&condFuel, NULL);

    for (int i=0; i<THREAD_NUM; i++) {
        if (i%2 == 0) {
            if (pthread_create(&th[i],NULL, &fuel_filling, NULL) != 0) {
                perror("Failed to create thread for fuel filling");
            }
        }
        else {
            if (pthread_create(&th[i],NULL, &car, NULL) != 0) {
                perror("Failed to create thread for car");
            }
        }
    }

    for (int i=0; i<THREAD_NUM; i++) {
        if (pthread_join(th[i],NULL) != 0) {
            perror("Failed to join thread");
        }
    }

    pthread_mutex_destroy(&mutexFuel);
    pthread_cond_destroy(&condFuel);
    return 0;
}