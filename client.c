#include <stdio.h> 
#include <string.h> 
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 

// Constants
#define SOCKET_ERROR -1
#define ERROR 1
#define MESSAGE_SIZE 1024
#define PORT 8081

int main(int argc, char* argv[]) {
    // Declare variables
    char clientMessage[MESSAGE_SIZE]; // Holds file data to be sent
    char* file_path = "/Users/steven/Documents/Year-4/Systems-Software/Assignment2/";
    char* file_name = argv[1];
    char* save_path = argv[2];

    // Check for correct number of command line args
    if(argc != 3) {
        puts("Error: Incorrect number of args");
        puts("Usage: ./client file_name file_path");

        return ERROR;
    }

    // Initialise client variables
    int SID; 
    struct sockaddr_in server;
    char serverMessage[MESSAGE_SIZE];

    // Create a socket 
    SID = socket(AF_INET, SOCK_STREAM, 0);
    
    // Check if socket created successfully
    if(SID == SOCKET_ERROR) {
        puts("Could not create socket");

        return ERROR;
    } else {
        puts("Socket created successfully");
    }

    // Set sockaddr_in variables
    server.sin_port = htons(PORT); // Set protocol for communication
    server.sin_family = AF_INET; // Use IPv4 protocol
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to server
    if(connect(SID, (struct sockaddr *)&server, sizeof(server)) < 0) {
        puts("Connection failed");

        return ERROR;
    }

    puts("Successfully connected to server");

    char* fs_name = (char *) malloc(1 + strlen(file_path) + strlen (file_name));
    
    // Create filename and path to retrieve file
    strcpy(fs_name, file_path);
    strcat(fs_name, file_name);

    printf("Sending %s to the %s directory on the server...\n", file_name, save_path);

    // Copy file name and path to message
    bzero(clientMessage, MESSAGE_SIZE);
    strcpy(clientMessage, file_name);

    // Send filename to be saved to server and check if it sends correctly
    if(send(SID, clientMessage, sizeof(clientMessage), 0) == SOCKET_ERROR) {
        fprintf(stderr, "ERROR: Failed to send file %s\n", fs_name);

        return ERROR;
    }

    // Copy file name and path to message
    bzero(clientMessage, MESSAGE_SIZE);
    strcpy(clientMessage, save_path);

    // Send path to save to server and check if it sends correctly
    if(send(SID, clientMessage, sizeof(clientMessage), 0) == SOCKET_ERROR) {
        fprintf(stderr, "ERROR: Failed to send path %s\n", save_path);

        return ERROR;
    }

    // Open the file in read mode
    FILE* fs = fopen(file_name, "r");

    // Check file opened correctly
    if(fs == NULL) {
        printf ("ERROR: File %s not found\n", fs_name);
        
        return ERROR;
    }

    bzero(clientMessage, MESSAGE_SIZE);

    // Get data from the file
    while((fgets(clientMessage, MESSAGE_SIZE, fs)) != NULL) {
        // Send the data to the server
        if(send(SID, clientMessage, sizeof(clientMessage), 0) == SOCKET_ERROR) {
            fprintf(stderr, "ERROR: Failed to send file %s\n", fs_name);

            return ERROR;
        }

        bzero(clientMessage, MESSAGE_SIZE);
    }

    puts("File sent successfully");

    // Receive reply from server
    bzero(serverMessage, MESSAGE_SIZE);
    if(recv(SID, serverMessage, MESSAGE_SIZE, 0) < 0) {
        puts("I/O error");
        
        return ERROR;
    } 

    printf("Server sent: %s", serverMessage);
    close(SID);

    return 0;
}