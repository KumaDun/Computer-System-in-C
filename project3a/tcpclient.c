#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#define MAX_LINE 20

int main (int argc, char *argv[]) {
  char* host_addr = argv[1];
  int port = atoi(argv[2]);
  int xNum = atoi(argv[3]);
  int yNum;
  int zNum;

  /* Open a socket */
  int client_socket;
  if((client_socket = socket(PF_INET, SOCK_STREAM, 0)) <0){
    perror("simplex-talk: socket");
    close(client_socket);
    exit(1);
  }

  /* Config the server address */
  struct sockaddr_in sin;
  sin.sin_family = AF_INET; 
  sin.sin_addr.s_addr = inet_addr(host_addr);
  sin.sin_port = htons(port);
  // Set all bits of the padding field to 0
  memset(sin.sin_zero, '\0', sizeof(sin.sin_zero));

  /* Connect to the server */
  //need to cast to sockaddr ptr
  if(connect(client_socket, (struct sockaddr *)&sin, sizeof(sin))<0){
    perror("simplex-talk: connect");
    close(client_socket);
    exit(1);
  }

  /*main loop: get and send lines of text */

  //SEND HELLO X
  //prepare client message
  //printf("prepare client message \n");
  char client_sent[MAX_LINE];
  memset(client_sent,'\0',MAX_LINE);
  sprintf(client_sent,"HELLO %d\n",xNum);

  //send client message to server
  //printf("send client message to server 1st time \n");
  int len = strlen(client_sent)+1;
  ///printf("length of client sent 1st time is %d\n", len);
  send(client_socket, client_sent, len, 0);


  //RECEIVE HELLO Y
  //receive server response
  //printf("receive server response 1st time \n");
  char server_response[MAX_LINE];
  memset(server_response,'\0',MAX_LINE);
  if (len = recv(client_socket, &server_response, sizeof(server_response), 0) < 0) {
    perror("error in Receive server response 1st time \n");
    close(client_socket);
    exit(1);
  }
  ///printf("length of received server response 1st time is %d\n", len);
  if (sscanf(server_response,"HELLO %d",&yNum)<0) {
    perror("didn't get number from server \n");
    close(client_socket);
    exit(1);
  }

  //printf("check if server response is valid \n");
  //check if server response is valid
  if (yNum!=xNum+1) {
    perror("Error");
    close(client_socket);
    exit(1);
  }
  //print server response
  fputs(server_response,stdout);
  fputc('\n',stdout);
  fflush(stdout);


  //SEND HELLO Z
  //prepare new client message
  zNum = yNum+1;
  memset(client_sent,'\0',MAX_LINE);
  sprintf(client_sent,"HELLO %d",zNum);
  len = strlen(client_sent)+1;
  ///printf("length of client sent 2nd time is %d\n", len);
  send(client_socket,client_sent,len,0);

  //finish program close socket
  close(client_socket);
  
  return 0;
}
