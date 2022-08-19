#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#define MAX_LINE 20

int main() {
    //create a socket
    int network_socket;
    network_socket = socket(AF_INET, SOCK_STREAM,0);

    //specify an address for the socket
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9002);
    server_address.sin_addr.s_addr = INADDR_ANY;

    int connection_status = connect (network_socket, (struct sockaddr *) &server_address , sizeof(server_address));
    //check for error with the connection
    if (connection_status == -1) {
        printf("There was an error making a connection to the remote socket \n \n");
    }

    //receive data from the server
    char server_reponse[MAX_LINE];
    recv(network_socket, &server_reponse, sizeof(server_reponse), 0);

    //print out the server's response
    printf("The client server sent the data, %s\n", server_reponse);

    //and then close the socket
    close(network_socket);

    return (0);




}