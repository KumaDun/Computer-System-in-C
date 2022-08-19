#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>

#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

/* This is a reference socket server implementation that prints out the messages
 * received from clients. */

#define MAX_PENDING 100
#define MAX_LINE 50

void handle_first_shake(int new_socket, int * xNumPtr);
void handle_second_shake(int new_socket, int xNum);

int main(int argc, char *argv[]) {
    int port = atoi(argv[1]);
    /*setup passive open*/
    int server_socket;
    if((server_socket = socket(PF_INET, SOCK_STREAM, 0)) <0) {
        perror("simplex-talk: socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    /* Config the server address */
    //printf("Config the server address \n");
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    sin.sin_family = AF_INET; 
    sin.sin_addr.s_addr = inet_addr("127.0.0.1");
    sin.sin_port = htons(port);
    // Set all bits of the padding field to 0
    memset(sin.sin_zero, '\0', sizeof(sin.sin_zero));

    /* Bind the socket to the address */
    //printf("bind \n");
    if((bind(server_socket, (struct sockaddr*)&sin, sizeof(sin)))<0) {
        perror("simplex-talk: bind");
        exit(EXIT_FAILURE);
    }
    //printf("listen \n");
    // connections can be pending if many concurrent client requests
    listen(server_socket, MAX_PENDING);  

    fd_set rfds;
    fd_set rfds_sever;
    fd_set temp_rfds;
    fd_set temmp_rfds_server;

    FD_ZERO(&rfds);
    FD_ZERO(&rfds_sever);
    FD_ZERO(&temp_rfds);
    FD_ZERO(&temmp_rfds_server);    

    int maxfd = server_socket;

    FD_SET(server_socket,&rfds_sever);
    FD_SET(server_socket,&rfds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 500;

    int new_sockets[MAX_PENDING];
    memset(new_sockets,-1,sizeof(new_sockets));
    int new_sockets_i = 0;
    int handshake_flags[MAX_PENDING];
    int handshake_xNums[MAX_PENDING];
    memset(handshake_flags,-1,sizeof(handshake_flags));

    while (1) {
        //printf("In the loop \n");
        //each time need to copy temp rfds since select will change unchanged file descriptor 0
        temp_rfds = rfds;
        //temmp_rfds_server = rfds_sever;
        //select all file set
        int res_select = select(maxfd+1,&temp_rfds,NULL,NULL,NULL);
        //select only server set
        //int res_select_sever = select(maxfd+1,&temmp_rfds_server,NULL,NULL,&timeout);

        //select error
        if (res_select < 0) {
            perror("select");
            exit(EXIT_FAILURE);
        }
        //out of time interval, impossible in this case since NULL timeout
        else if (res_select == 0){
            continue;
        }
        //some file ready
        else {
            //printf("Some file ready \n");
            //handle situation if server socket ready
            if (FD_ISSET(server_socket,&temp_rfds)) {
                //printf("Handle server_socket ready, accept \n");
                int new_socket = accept(server_socket, (struct sockaddr *) &sin, &len);
                if (new_socket < 0) {
                    perror ("simplex-talk: accept");
                    close(new_socket);
                    exit(EXIT_FAILURE);
                }
                else {
                    new_sockets[new_sockets_i] = new_socket;
                    //unblock new socket
                    fcntl(new_sockets[new_sockets_i],F_SETFL,O_NONBLOCK);
                    //add new socket to rfds
                    FD_SET(new_sockets[new_sockets_i],&rfds);
                    //update maxfd
                    maxfd = new_sockets[new_sockets_i]>maxfd? new_sockets[new_sockets_i]:maxfd;
                    new_sockets_i ++;   
                }
            }
            //handle new socket ready
            for (int i = 0; i<MAX_PENDING;i++) {
                if (new_sockets[i]>0 && FD_ISSET(new_sockets[i],&temp_rfds)){
                    //handle second shake
                    if (handshake_flags[i] > -1) {
                        handle_second_shake(new_sockets[i],handshake_xNums[i]);
                        //after second handler, remove the fd from set
                        FD_CLR(new_sockets[i],&rfds);
                        close(new_sockets[i]);
                        new_sockets[i] = -1;
                    }
                    //handle first shake
                    if (handshake_flags[i] == -1){
                        handle_first_shake(new_sockets[i],&handshake_xNums[i]);
                        //printf("1st handler handshake_nums[%d] is %d \n", i, handshake_xNums[i]);
                        handshake_flags[i] = 0;
                    }
                    
                }
            }
        }
    }
    return 0; 
}

//receive HELLO X and send HELLO Y
void handle_first_shake (int new_socket, int * xNumPtr) {
    //printf("In handle_first_shake \n");
    char client_message[MAX_LINE];
    char server_response[MAX_LINE];
    int messagelen;
    int xNum;
    int yNum;
    //RECEIVE HELLO X
    if (messagelen = recv(new_socket,client_message,sizeof(client_message),0)<0) {
        perror("receive xNum recv function error");
        close(new_socket);
        exit(EXIT_FAILURE);
    }
    client_message[MAX_LINE -1] = '\0';

    if(sscanf(client_message,"HELLO %d",&xNum)<0) {
        perror("did not receive xNum");
        close(new_socket);
        exit(EXIT_FAILURE);
    }

    fputs(client_message,stdout);
    fputc('\n',stdout);
    fflush(stdout);

    *xNumPtr = xNum;

    //SEND HELLO Y
    yNum = xNum + 1;
    memset(server_response,'\0',MAX_LINE);
    sprintf(server_response,"HELLO %d",yNum);
    messagelen = strlen(server_response)+1;
    send(new_socket,server_response,messagelen,0);

}

//receive HELLO Z
void handle_second_shake (int new_socket, int xNum) {
    //printf("Entered second handler \n");
    char client_message[MAX_LINE];
    int messagelen;
    int zNum;
    memset(client_message,'\0',MAX_LINE);
    if (messagelen = recv(new_socket,client_message,sizeof(client_message),0)<0){
        perror("receive zNum error");
        close(new_socket);
        exit(EXIT_FAILURE);
    }
    client_message[MAX_LINE-1] = '\0';

    if (sscanf(client_message,"HELLO %d", &zNum)<0) {
        perror("didnt get number from client 2nd time");
        close(new_socket);
        exit(EXIT_FAILURE);
    }
    if (zNum != xNum+2){
        perror("zNum != xNum + 2");
        close(new_socket);
        exit(EXIT_FAILURE);
    }

    fputs(client_message,stdout);
    fputc('\n',stdout);
    fflush(stdout);

}