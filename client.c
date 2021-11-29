#include <stdio.h> 
#include <string.h> 
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <pwd.h>

// Constants
#define SOCKET_ERROR -1
#define ERROR 1
#define MESSAGE_SIZE 1024
#define PORT 8081

// Function declarations
void send_username();
void send_filedata(char* fs_name);
void send_data(char* data);

// Initialise client variables
int SID; 
char clientMessage[MESSAGE_SIZE]; // Holds file data to be sent

int main(int argc, char* argv[]) {
    // Declare variables
    char* file_path = "/Users/steven/Documents/Year-4/Systems-Software/Assignment2/Upload/";
    char* file_name = argv[1];
    char* save_path = argv[2];
    struct sockaddr_in server;
    char serverMessage[MESSAGE_SIZE]; // Store data received from server

    // Check for correct number of command line args
    if(argc != 3) {
        puts("Error: Incorrect number of args");
        puts("Usage: ./client file_name file_path");

        return ERROR;
    }

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

    // Connect to the server
    if(connect(SID, (struct sockaddr *)&server, sizeof(server)) < 0) {
        puts("Connection failed");

        return ERROR;
    }

    puts("Successfully connected to server");

    send_username();
    send_data(file_name);
    send_data(save_path);

    // Create filename and path to retrieve file
    char* fs_name = (char *) malloc(1 + strlen(file_path) + strlen (file_name));
    strcpy(fs_name, file_path);
    strcat(fs_name, file_name);

    printf("Sending %s to the %s directory on the server...\n", file_name, save_path);
    send_filedata(fs_name);

    puts("File sent to server successfully");

    // Receive reply from server
    bzero(serverMessage, MESSAGE_SIZE);

    // Receive data from server and check if received without error
    if(recv(SID, serverMessage, MESSAGE_SIZE, 0) < 0) {
        puts("I/O error");
        
        return ERROR;
    } 

    printf("Server sent: %s", serverMessage);
    close(SID);

    return 0;
}

/**
 * Function that retrieves a users username and 
 * sends the username to the server
 */
void send_username() {
    struct passwd* pw;

    // Check for error
    if((pw = getpwuid( getuid())) == NULL) {
        puts("ERROR: Failed to retrieve username");

       exit(ERROR);
    }

    char* username = pw->pw_name;

    send_data(username);

    return;
}

/**
 * Function that sends a files data to the server
 * Function open a file in read mode, stores the data
 * in an array, and sends it to the server
 */
void send_filedata(char* fs_name) {
    // Open the file in read mode
    FILE* fs = fopen(fs_name, "r");

    // Check file opened correctly
    if(fs == NULL) {
        printf("ERROR: File %s not found\n", fs_name);
        
        exit(ERROR);
    }

    bzero(clientMessage, MESSAGE_SIZE);

    // Get data from the file
    while((fgets(clientMessage, MESSAGE_SIZE, fs)) != NULL) {
        // Send the data to the server and check if it sends correctly
        if(send(SID, clientMessage, sizeof(clientMessage), 0) == SOCKET_ERROR) {
            printf("ERROR: Failed to send file %s\n", fs_name);

            exit(ERROR);
        }

        bzero(clientMessage, MESSAGE_SIZE);
    }

    fclose(fs);
}

/**
 * Function that sends generic data held in an array to the server
 */
void send_data(char* data) {   
    // Erase data in message to be sent and copy new data in
    bzero(clientMessage, MESSAGE_SIZE);
    strcpy(clientMessage, data);

    // Send data to be saved to server and check if it sends correctly
    if(send(SID, clientMessage, sizeof(clientMessage), 0) == SOCKET_ERROR) {
        printf("ERROR: Failed to send data to server: %s\n", data);

        exit(ERROR);
    }

    return;
}