#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#define MAX_LINE 25

int main() {
    char server_message [MAX_LINE] = "Reached the server!\n";

    int server_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    // define the server address
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(9002);

    // bind the socket to our specified IP and port
    bind(server_socket,(struct sockaddr *) &server_address, sizeof(server_address));

    // second parameter is how many connections can be waiting for the socket
    listen(server_socket,1);
    
    int client_socket;
    client_socket = accept(server_socket,NULL,NULL);

    // send the message
    send(client_socket,server_message,sizeof(server_message),0);

    //close the socket 
    close(server_socket);

    return 0;
}