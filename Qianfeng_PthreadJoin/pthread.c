#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>


void *thread_fun1 (void* arg) {
    int i;
    //str is going to return to main, so str address cannot be free or released otherwise pthread_join cannot catch it
    char * str = "hello world";
    printf("%s\n",(char*) arg);
    for (i =0;i<5;i++) {
        printf("this is thread1\n");
        sleep(1);
    }
    return str;

}

void *thread_fun2 (void* arg) {
    int i;
    printf("%d\n",*((int*) arg));
    for (i =0;i<5;i++){
        printf("this is thread2\n");
        sleep(1);
    }
    return NULL;

}

int main(int argc, char* argv[]) {
    pthread_t tid1, tid2;
    int ret;
    void * threadRet; 
    ret = pthread_create(&tid1, NULL, thread_fun1, "hello world");
    if (ret!=0) {
        perror("pthread_created 1 failed \n");
        return 0;
    }
    ret = pthread_create(&tid2, NULL, thread_fun2, "hello world");
    if (ret!=0) {
        perror("pthread_created 2 failed \n");
        return 0;
    }
    ret = pthread_join(tid1, &threadRet);
    if (ret != 0) {
        perror("pthread_1 join failed");
        return 0;
    }
    printf("thread1 end, return : %s \n", (char *) threadRet);
    ret = pthread_join(tid2, &threadRet);
    if (ret != 0) {
        perror("pthread_2 join failed");
        return 0;
    }
    return 0;
}