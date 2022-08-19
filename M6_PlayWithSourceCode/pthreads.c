#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdio.h>

void* myturn(void * arg);
void yourturn();


void* myturn(void * arg) {
    while(1)
    {
        sleep(1);
        printf("My Turn! \n");
    }
    return NULL;
}

void yourturn() {
    while(1)
    {
        sleep(1);
        printf("Your Turn! \n");
    }
}

int main() {
    pthread_t newthread;
    pthread_create(&newthread,NULL,myturn,NULL);
    pthread_join(newthread,NULL);

    //myturn();
    yourturn();
}