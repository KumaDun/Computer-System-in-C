#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

/* This is a reference socket server implementation that prints out the messages
 * received from clients. */

#define MAX_PENDING 20
#define MAX_LINE 20

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
  socklen_t len = sizeof(sin);
  int xNum;
  int yNum;
  int zNum;
  char client_message[MAX_LINE];
  char server_response[MAX_LINE];
  
  while(1){


    //RECEIVE HELLO X
    //accept client socket
    //printf("accept \n");
    if((new_s = accept(server_socket, (struct sockaddr *)&sin, &len)) <0){
      perror("simplex-talk: accept");
      close(new_s);
      exit(1);
    }
    memset(client_message,'\0',MAX_LINE);
    //receive client socket for 1st time
    //printf("receive client message 1st time\n");
    if (len = recv(new_s, client_message, sizeof(client_message), 0)<0){
      perror("receive error 1st time");
      close(new_s);
      exit(1);
    }
    ///printf("length of received client message 1st time is %d\n", len);
  
    client_message[MAX_LINE-1] = '\0';
    //check if client message is valid 1st time
    //printf("check client message 1st time \n");
    if (sscanf(client_message,"HELLO %d",&xNum)<0) {
      perror("didnt get number from client 1st time");
      close(new_s);    
      exit(1);
    }
    //print client message 1st time
    fputs(client_message,stdout);
    fputc('\n',stdout);
    fflush(stdout);

    

    //SEND HELLO Y
    //prepare server response
    yNum = xNum+1;
    memset(server_response,'\0',MAX_LINE);
    sprintf(server_response,"HELLO %d",yNum);
    len = strlen(server_response)+1;
    ///printf("length of server sent 1st time is %d\n", len);
    send(new_s, server_response, len, 0);


    //RECEIVE HELLO Z
    //receive client for 2nd time
    //printf("receive client message 2nd time\n");
    memset(client_message,'\0',MAX_LINE);
    if (len = recv(new_s,client_message,sizeof(client_message),0)<0){
      perror("receive error 2nd time");
      close(new_s);
      exit(1);
    }
    ///printf("length of received client message 2nd time is %d\n", len);
    client_message[MAX_LINE-1] = '\0';
    
    //check if client message is valid 2nd time
    //printf("check client message 2nd time\n");
    if (sscanf(client_message,"HELLO %d",&zNum)<0) {
      perror("didnt get number from client 2nd time");
      close(new_s);
      exit(1);
    }
    if (zNum != yNum + 1) {
      perror("Error");
      close(new_s);
      exit(1);
    }
    //print client message 2nd time
    fputs(client_message,stdout);
    fputc('\n',stdout);
    fflush(stdout);

    close(new_s);
  }

  close(server_socket);

  return 0;
}
