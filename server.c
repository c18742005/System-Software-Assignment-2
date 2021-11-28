#include <stdio.h> 
#include <string.h> 
#include <stdlib.h>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <pthread.h>
#include <stdbool.h>

// Constants
#define TRUE 1
#define SOCKET_ERROR -1
#define MESSAGE_SIZE 4096
#define PORT 8081
#define SERVER_BACKLOG 100

void* handle_connection(void* client_socket);

int main(int argc, char* argv[]) {
    // Initialise server variables
    int s; // socket descriptor 
    int cs; // client socket
    int connSize; // size of struct

    struct sockaddr_in server, client;

    // Create a socket 
    s = socket(AF_INET, SOCK_STREAM, 0);
    
    if(s == SOCKET_ERROR) {
        puts("Failed to create socket");
    } else {
        puts("Socket created successfully");
    }

    // Set sockaddr_in variables
    server.sin_port = htons(PORT); // Set protocol for communication
    server.sin_family = AF_INET; // Use IPv4 protocol
    server.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket 
    if(bind(s, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Bind error");
    } else {
        puts("Bind complete");
    }

    // Listen for a connection
    if(listen(s, SERVER_BACKLOG) < 0) {
        perror("Listen error");
    }

    while (true) {
        // Await connections
        puts("Awaiting incoming connection from client >> ");
        connSize = sizeof(struct sockaddr_in);

        // Accept a connection
        if((cs = accept(s, (struct sockaddr *)&client, (socklen_t *)&connSize)) < 0) {
            perror("Can't establish a connection");
        } else {
            puts("Connection from client accepted");
        }

        // keep track of thread
        pthread_t t;
        int *p_client = malloc(sizeof(int));
        *p_client = cs;

        pthread_create(&t, NULL, handle_connection, p_client);

    }

    return 0;
}

void * handle_connection(void* p_client_socket) {
    char message[MESSAGE_SIZE];
    int READSIZE; // size of sockaddr_in for client connection

    int client_socket = *((int*)p_client_socket);
    free(p_client_socket); // no longer needed

    // Read data from client
    while(1) {
        memset(message, 0, 500);
        READSIZE = recv(client_socket, message, 2000, 0);

        if(READSIZE == 0) {
            puts("Client disconnected");
            fflush(stdout);
        } else if(READSIZE == SOCKET_ERROR){
            perror("Read error");
            return NULL;
        }

        printf("Client said: %s\n", message);
        write(client_socket, "What? ", strlen("What? "));
    }

    close(client_socket);
    puts("Closing connection");
    return NULL;
}