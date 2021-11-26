#include <stdio.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 

// Constants
#define TRUE 1
#define FALSE 0
#define MESSAGE_SIZE 500
#define PORT 8081

int main(int argc, char* argv[]) {
    // Initialise client variables
    int SID; 
    struct sockaddr_in server;
    char clientMessage[MESSAGE_SIZE];
    char serverMessage[MESSAGE_SIZE];

    // Create a socket 
    SID = socket(AF_INET, SOCK_STREAM, 0);
    
    if(SID == -1) {
        puts("Could not create socket");
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

        return 1;
    }

    puts("Successfully connected to server");

    // Communicate with server
    while(TRUE) {
        puts("Enter message: ");
        scanf("%s", clientMessage);
        
        // Send data
        if(send(SID, clientMessage, strlen(clientMessage), 0) < 0) {
            puts("Sending data failed");

            return 1;
        }

        // Receive reply from server
        if(recv(SID, serverMessage, MESSAGE_SIZE, 0) < 0) {
            puts("I/O error");
            
            break;
        } 

        puts("\nServer sent: ");
        puts(serverMessage);
    }

    close(SID);

    return 0;
}