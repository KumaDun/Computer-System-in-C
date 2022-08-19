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
  //change it to 127.0.0.1
  char* host_addr = "127.0.0.1"; 
  int port = atoi(argv[1]);

  /*setup passive open*/
  int s;
  if((s = socket(PF_INET, SOCK_STREAM, 0)) <0) {
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

  /* Bind the socket to the address */
  if((bind(s, (struct sockaddr*)&sin, sizeof(sin)))<0) {
    perror("simplex-talk: bind");
    exit(1);
  }

  // connections can be pending if many concurrent client requests
  listen(s, MAX_PENDING);  

  /* wait for connection, then receive and print text */
  int new_s;
  socklen_t len = sizeof(sin);
  char buf[MAX_LINE];
  while (1){
    if((new_s = accept(s, (struct sockaddr *)&sin, &len)) <0){
      perror("simplex-talk: accept");
      exit(1);
    }
    
  //if this is a valid input sent from client side (1st-2nd HANDSHAKE)
  if (len = recv(new_s, buf, sizeof(buf), 0)>0) {
      fputs(buf, stdout);
      fputc('\n', stdout);
      fflush(stdout);

      char* token = strtok(buf, " ");
      token = strtok(NULL, " ");


      //convert the string into integer and increment by 1
      int x = atoi(token);
      int y = x + 1;

      char message[MAX_LINE];
      char y_string[MAX_LINE];
      strcpy(message, "HELLO ");
      sprintf(y_string, "%d", y);
      strcat(message, y_string); 
      len = strlen(message) + 1;
      send(new_s, message , len, 0);
  }

  //2nd to 3rd HANDSHAKE
  if (len = recv(new_s, buf, sizeof(buf), 0)>0) {
      fputs(buf, stdout);
      fputc('\n', stdout);
      fflush(stdout);
  }

  //END HANDSHAKE
  close(new_s);
  }

    return 0;
}
