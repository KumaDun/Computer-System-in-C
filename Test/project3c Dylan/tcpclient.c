#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#define MAX_LINE 1000


int main (int argc, char *argv[]) {
  char* host_addr = argv[1];
  int port = atoi(argv[2]);

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
  char buf[MAX_LINE];
  const char space[2] = " ";
  int num;

  memset(buf,0,strlen(buf));
    sprintf(buf, "Hello %s", argv[3]);
    write(s, buf, strlen(buf)+1);

  memset(buf,0,strlen(buf));
  if(read(s, buf, sizeof(buf))){
        char *tok = strtok(buf, space);
        for(;;tok = strtok(NULL, space)){
              if(tok == NULL){
                printf("HELLO %d\n",num);
                fflush(stdout);
                if(num!=(atoi(argv[3])+1)){
                  perror("Invalid");
                  close(s);
                  exit(EXIT_FAILURE);
                }
                char buf2[MAX_LINE];
                memset(buf2,0,strlen(buf2));
                sprintf(buf2, "Hello %d", num+1);
                write(s, buf2, strlen(buf2)+1);
                break;
              }
              else{
              num = atoi(tok);
            }
          }
  }

}
