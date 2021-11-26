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
    // Initialise server variables
    int s; // socket descriptor 
    int cs; // client socket
    int connSize; // size of struct
    int READSIZE; // size of sockaddr_in for client connection

    struct sockaddr_in server, client;
    char message[MESSAGE_SIZE];

    // Create a socket 
    s = socket(AF_INET, SOCK_STREAM, 0);
    
    if(s == -1) {
        puts("Could not create socket");
    } else {
        puts("Socket created successfully");
    }

    // Set sockaddr_in variables
    server.sin_port = htons(PORT); // Set protocol for communication
    server.sin_family = AF_INET; // Use IPv4 protocol
    server.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket 
    if(bind(s, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Bind issue");
        return 1;
    } else {
        puts("Bind complete");
    }

    // Listen for a connection
    listen(s, 3);

    // Await connections
    puts("Awaiting incoming connection from client >> ");
    connSize = sizeof(struct sockaddr_in);

    // Accept a connection
    cs = accept(s, (struct sockaddr *)&client, (socklen_t *)&connSize);

    if(cs < 0) {
        perror("Can't establish a connection");
        return 1;
    } else {
        puts("Connection from client accepted");
    }

    // Read data from client
    while(1) {
        memset(message, 0, 500);
        READSIZE = recv(cs, message, 2000, 0);

        if(READSIZE == 0) {
            puts("Client disconnected");
            fflush(stdout);
        } else if(READSIZE == -1){
            perror("Read error");
            return 1;
        }

        printf("Client said: %s\n", message);
        write(cs, "What? ", strlen("What? "));
    }

    return 0;
}