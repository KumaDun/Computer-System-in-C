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
  char* command = argv[3];
  /* Open a socket */
  int s;
  if((s = socket(PF_INET, SOCK_STREAM, 0)) <0){
    perror("simplex-talk: socket");
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
  if(connect(s, (struct sockaddr *)&sin, sizeof(sin))<0){
    perror("simplex-talk: connect");
    close(s);
    exit(1);
  }

  /*main loop: get and send lines of text */
  char buf [MAX_LINE];
  
  //add HELLO at the beginning of the string and concat. string
  strcpy(buf, "HELLO ");
  strcat(buf, command); 

  //convert the string into numerical intenger
  int x = atoi(command);

  int len = strlen(buf)+1;
  send(s, buf, len, 0);

  //create y
  char y[MAX_LINE];

  //get y from the server side
  recv(s, y, sizeof(y),0);
  fputs(y, stdout);
  fputc('\n', stdout);
  fflush(stdout);

  //send z to server side
  char* token = strtok(y, " ");
  token = strtok(NULL, " ");

  int y_int = atoi(token);
  int z = y_int + 1;
  char z_string[MAX_LINE];
  strcpy(buf, "HELLO ");
  sprintf(z_string, "%d", z);
  strcat(buf, z_string); 
  len = strlen(buf) + 1;
  send(s, buf , len, 0);
  
}
