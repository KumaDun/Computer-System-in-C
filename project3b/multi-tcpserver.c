#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

/* This is a reference socket server implementation that prints out the messages
 * received from clients. */

#define MAX_PENDING 20
#define MAX_LINE 20
#define MAX_THREAD 40

void * three_shake_hands (void *);



int main(int argc, char *argv[]) {
  int port = atoi(argv[1]);

  /*setup passive open*/
  int server_socket;
  if((server_socket = socket(PF_INET, SOCK_STREAM, 0)) <0) {
    perror("simplex-talk: socket");
    close(server_socket);
    exit(1);
  }

  /* Config the server address */
  //printf("Config the server address \n");
  struct sockaddr_in sin;
  sin.sin_family = AF_INET; 
  sin.sin_addr.s_addr = inet_addr("127.0.0.1");
  sin.sin_port = htons(port);
  // Set all bits of the padding field to 0
  memset(sin.sin_zero, '\0', sizeof(sin.sin_zero));

  /* Bind the socket to the address */
  //printf("bind \n");
  if((bind(server_socket, (struct sockaddr*)&sin, sizeof(sin)))<0) {
    perror("simplex-talk: bind");
    exit(1);
  }

  //printf("listen \n");
  // connections can be pending if many concurrent client requests
  listen(server_socket, MAX_PENDING);  

  /* wait for connection, then receive and print text */
  int new_s;
  int thread_idx=0;
  socklen_t len = sizeof(sin);
  
  pthread_t thread_id [MAX_THREAD];
  pthread_attr_t pthreadattr;
  pthread_attr_init(&pthreadattr);
  pthread_attr_setdetachstate(&pthreadattr, PTHREAD_CREATE_DETACHED);
  while(1){
    //RECEIVE HELLO X
    //accept client socket
    //printf("accept \n");
    if((new_s = accept(server_socket, (struct sockaddr *)&sin, &len)) <0){
      perror("simplex-talk: accept");
      close(new_s);
      exit(1);
    }
    else {
      
      pthread_create(&thread_id[thread_idx], NULL, &three_shake_hands, (void*) &new_s);
      thread_idx ++;
      //printf("thread_idx is %d \n", thread_idx);
    }
  }
  pthread_attr_destroy(&pthreadattr);
  close(server_socket);

  return 0;
}

void * three_shake_hands (void * new_s) {
  int new_socket = *(int*) new_s;
  char client_message[MAX_LINE];
  int messagelen;
  int xNum;
  int yNum;
  int zNum;
  char server_response[MAX_LINE];
  memset(client_message,'\0',MAX_LINE);
  //receive client socket for 1st time
  //printf("receive client message 1st time\n");
  
  //RECEIVE HELLO X
  //receive client socket for 1st time
  if (messagelen = recv(new_socket,client_message,sizeof(client_message),0)<0) {
    perror("receive xNum error");
    close (new_socket);
    exit(1);
  }
  ///printf("length of received client message 1st time is %d\n", len);
  
  client_message[MAX_LINE-1] = '\0';
  //check if client message is valid 1st time
  //printf("check client message 1st time \n");

  if(sscanf(client_message,"HELLO %d",&xNum)<0) {
    perror("receive error 1st time");
    close(new_socket);
    exit(1);
  }

  fputs(client_message,stdout);
  fputc('\n',stdout);
  fflush(stdout);

  //SEND HELLO Y
  //prepare server response
  yNum = xNum+1;
  memset(server_response,'\0',MAX_LINE);
  sprintf(server_response,"HELLO %d", yNum);
  messagelen = strlen(server_response)+1;
  send(new_socket,server_response,messagelen,0);

  //RECEIVE HELLO Z
  //receive client for 2nd time
  memset(client_message,'\0',MAX_LINE);
  if (messagelen = recv(new_socket,client_message,sizeof(client_message),0)<0){
    perror("receive zNum error");
    close(new_socket);
    exit(1);
  }
  client_message[MAX_LINE-1] = '\0';

  if (sscanf(client_message,"HELLO %d", &zNum)<0) {
    perror("didnt get number from client 2nd time");
    close(new_socket);
    exit(1);
  }

  if (zNum != yNum+1){
    perror("Error");
    close(new_socket);
    exit(1);
  }
  //print client message 2nd time
  fputs(client_message,stdout);
  fputc('\n',stdout);
  fflush(stdout);

  close(new_socket);
  
}