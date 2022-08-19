#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <pthread.h>


/* This is a reference socket server implementation that prints out the messages
 * received from clients. */

#define MAX_PENDING 50
#define MAX_LINE 100

void handle_second_shake(int i,int num,const char space[2]);
void handle_first_shake(int i ,int num,const char space[2]);

int main(int argc, char *argv[]) {
  char* host_addr = "127.0.0.1";
  int port = atoi(argv[1]);

  /*setup passive open*/
  int s;
  if((s = socket(AF_INET, SOCK_STREAM, 0)) <0) {
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
  //listen(s, MAX_PENDING);

  if(listen(s, MAX_PENDING)<=-1){
    perror("Invalid s");
    close(s);
    exit(EXIT_FAILURE);
  }

  /* wait for connection, then receive and print text */
  int new_s;
  socklen_t len = sizeof(sin);
  //char buf[MAX_LINE];

  //pthread_t tids[MAX_LINE];
  int *id;


/**/
  struct timeval t;
  t.tv_sec = 5;
  t.tv_usec = 0;

  struct sockaddr_in name;
  fd_set a;
  fd_set b;

  FD_ZERO(&a);
  FD_SET(s,&a);

  int client[MAX_LINE];

  memset(client,-1,sizeof(client));
  /**/
  const char space[2] = " ";
  b = a;
  while(1) {
    if( select ( FD_SETSIZE, &b, NULL, NULL, NULL)>=0){
      int i =0;
      while(i<FD_SETSIZE)
      {
        int num=1; 
        if (FD_ISSET (i, &b)){
          if(i!=s){
            if(client[i] != -1){///2
              handle_second_shake(i,num,space);
              client[i]++;
              FD_CLR(i,&a);
              client[i]=-1;
              close(i);
            }
            else {//1
              handle_first_shake(i,num,space);
              client[i]++;
            }
         }
        else{
          int length = sizeof(name);
          new_s = accept(s,(struct sockaddr *) &name,&length);
          if(new_s>=0){
            FD_SET(new_s,&a);
          }
          else{
            perror("Invalid s");
            exit(EXIT_FAILURE);
          }
        }
        }
        i++;
      }
    }
    else {
      perror("Invalid s");
      exit(EXIT_FAILURE);
    }
    b = a;
  }
  close(new_s);
  free(id);
  return 0;
}



void handle_second_shake(int i,int num,const char space[2]){
  char buf[MAX_LINE];
  char *tok = strtok(buf, space);
  recv(i, buf, sizeof(buf),0);
  for(;;tok = strtok(NULL, space)){
        if(tok == NULL){
          printf("HELLO %d\n",num);
          fflush(stdout);
          break;
        }
        else{
        num = atoi(tok);
      }
    }
}

void handle_first_shake( int i,int num,const char space[2]){
  char buf[MAX_LINE];
  char* tok = strtok(buf, space);
  recv(i, buf, sizeof(buf),0);
  for(;;tok = strtok(NULL, space)){
    if(tok == NULL){
        printf("HELLO %d\n",num);
        fflush(stdout);
        num++;
        char buf2[MAX_LINE];
       memset(buf2,0,strlen(buf2));

        sprintf(buf2, "Hello %d", num);

        send(i, buf2, strlen(buf2)+1,0);
            break;
      }
      else{
      num = atoi(tok);
    }
  }

}
