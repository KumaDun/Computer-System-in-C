#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define THREAD_NUM 2

void* routine(void* args) {
    sleep(1);
    printf("Finished execution\n");
}

int main(int argc, char *argv[]) {
    pthread_t th[THREAD_NUM];
    pthread_attr_t pthreadattr;
    pthread_attr_init(&pthreadattr);
    pthread_attr_setdetachstate(&pthreadattr, PTHREAD_CREATE_DETACHED);
    
    int i;
    for (i = 0; i < THREAD_NUM; i++) {
        if (pthread_create(&th[i], &pthreadattr, &routine, NULL) != 0) {
            perror("Failed to create thread");
        }
        // pthread_detach(th[i]);
    }

    /*for (i = 0; i < THREAD_NUM; i++) {
        if (pthread_join(th[i], NULL) != 0) {
            perror("Failed to join thread");
        }
    }*/
    pthread_attr_destroy(&pthreadattr);
    pthread_exit(0);
}