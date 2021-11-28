#include <stdio.h> 
#include <string.h> 
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <pthread.h>

// Constants
#define SOCKET_ERROR -1
#define MESSAGE_SIZE 1024
#define PORT 8081
#define SERVER_BACKLOG 100

// Declare functions
void* handle_connection(void* client_socket);

int main(int argc, char* argv[]) {
    // Initialise server variables
    int s; // socket descriptor 
    int cs; // client socket
    int connSize; // size of struct
    struct sockaddr_in server, client;

    // Create a socket 
    s = socket(AF_INET, SOCK_STREAM, 0);
    
    // Check socket created successfully
    if(s == SOCKET_ERROR) {
        puts("Failed to create socket");
    } else {
        puts("Socket created successfully");
    }

    // Set sockaddr_in variables
    server.sin_port = htons(PORT); // Set protocol for communication
    server.sin_family = AF_INET; // Use IPv4 protocol
    server.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket and check it binds successfully
    if(bind(s, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Bind error");
    } else {
        puts("Bind complete");
    }

    // Listen for a connection
    if(listen(s, SERVER_BACKLOG) < 0) {
        perror("Listen error");
    }

    // Await connections
    while (true) {
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

        // Create the thread
        pthread_create(&t, NULL, handle_connection, p_client);
    }

    return 0;
}

/**
 * Function that handles client multiple client connections to the server
 */
void* handle_connection(void* p_client_socket) {
    int READSIZE; // size of sockaddr_in for client connection
    char* fr_path = "/Users/steven/Documents/Year-4/Systems-Software/Assignment2/upload/";
    char file_name[MESSAGE_SIZE];
    char save_path[MESSAGE_SIZE];
    char message[MESSAGE_SIZE];

    // Store client socket
    int client_socket = *((int*)p_client_socket);
    free(p_client_socket); // no longer needed

    // Received file name to upload from a client
    bzero(file_name, MESSAGE_SIZE);
    READSIZE = recv(client_socket, file_name, MESSAGE_SIZE, 0);

    // Check data received correctly
    if(READSIZE == 0) {
        puts("Client disconnected");
        fflush(stdout);
    } else if(READSIZE == SOCKET_ERROR) {
        perror("Read error");
        write(client_socket, "File upload failed\n", strlen("File upload failed\n"));

        return NULL;
    }

    // Received file path to upload from a client
    bzero(save_path, MESSAGE_SIZE);
    READSIZE = recv(client_socket, save_path, MESSAGE_SIZE, 0);

    // Check data received correctly
    if(READSIZE == 0) {
        puts("Client disconnected");
        fflush(stdout);
    } else if(READSIZE == SOCKET_ERROR) {
        perror("Read error");
        write(client_socket, "File upload failed\n", strlen("File upload failed\n"));

        return NULL;
    }

    // Store file location and name
    char* fr_name = (char*) malloc(2 + strlen(fr_path) + strlen(file_name));
    strcpy(fr_name, fr_path);
    strcat(fr_name, save_path);
    strcat(fr_name, "/");
    strcat(fr_name, file_name);

    // Open file for writing
    FILE *fr = fopen(fr_name, "w");

    // Check file opened successfully
    if(fr == NULL) {
        printf("File %s Cannot be opened on server\n", fr_name);
        write(client_socket, "File upload failed\n", strlen("File upload failed\n"));

        return NULL;
    } 

    // Receive file data from the client
    while(true) {
        READSIZE = recv(client_socket, message, MESSAGE_SIZE, 0);

        // Check data received correctly
        if(READSIZE == 0) {
            break;
        } else if(READSIZE == SOCKET_ERROR) {
            perror("Read error");
            write(client_socket, "File upload failed\n", strlen("File upload failed\n"));

            return NULL;
        }

        // Store data in file and check it stored correctly
        if((fprintf(fr, "%s", message)) < 0) {
            write(client_socket, "File upload failed\n", strlen("File upload failed\n"));
        }

        // Close file and update user on success
        fclose(fr);
        write(client_socket, "File uploaded successfully\n", strlen("File uploaded successfully\n"));
        bzero(message, MESSAGE_SIZE);
    }

    // Close client connection
    puts("Data written successfully\nClosing connection to client");
    close(client_socket);

    return NULL;
}