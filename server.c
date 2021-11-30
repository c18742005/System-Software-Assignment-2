#include <stdio.h> 
#include <string.h> 
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h> 
#include <sys/wait.h>
#include <arpa/inet.h> 
#include <unistd.h> 
#include <pthread.h>
#include <pwd.h>
#include <grp.h>

// Constants
#define SOCKET_ERROR -1
#define ERROR 1
#define MESSAGE_SIZE 1024
#define PORT 8081
#define SERVER_BACKLOG 50

// Declare functions
void* handle_connection(void* client_socket);

// Declare lock
pthread_mutex_t lock;

int main(int argc, char* argv[]) {
    // Initialise server variables
    int s; // socket descriptor 
    int cs; // client socket
    int connSize; // size of struct
    struct sockaddr_in server, client;

    // Initialise mutext lock for synchronisation
    if(pthread_mutex_init(&lock, NULL) != 0) {
        perror("Mutex init failed\n");

        return ERROR;
    }

    // Create a socket 
    s = socket(AF_INET, SOCK_STREAM, 0);
    
    // Check socket created successfully
    if(s == SOCKET_ERROR) {
        perror("Failed to create socket");

        return ERROR;
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

        return ERROR;
    } else {
        puts("Bind complete");
    }

    // Listen for a connection
    if(listen(s, SERVER_BACKLOG) < 0) {
        perror("Listen error");

        return ERROR;
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

        // keep track of threads
        pthread_t t;
        int *p_client = (int*) malloc(sizeof(gid_t));
        *p_client = cs;

        // Create a thread
        pthread_create(&t, NULL, handle_connection, p_client);
        pthread_join(t, NULL);
    }

    // Destroy the lock on completion
    pthread_mutex_destroy(&lock);

    return 0;
}

/**
 * Function that handles multiple client connections to the server
 */
void* handle_connection(void* p_client_socket) {
    int READSIZE; // size of sockaddr_in for client connection
    char* fr_path = "/var/www/html/Assignment2/";
    char username[MESSAGE_SIZE];
    char file_name[MESSAGE_SIZE];
    char save_path[MESSAGE_SIZE];
    char message[MESSAGE_SIZE];

    // Store client socket
    int client_socket = *((int*) p_client_socket);
    free(p_client_socket); // No longer needed

    // Apply lock
    printf("\nApplying lock");
    pthread_mutex_lock(&lock);

    // Received user name to upload from a client
    bzero(username, MESSAGE_SIZE);
    READSIZE = recv(client_socket, username, MESSAGE_SIZE, 0);

    // Check data received correctly
    if(READSIZE == 0) {
        puts("Client disconnected");
        fflush(stdout);

        //Remove mutex lock
        printf("\nRemoving lock");
        pthread_mutex_unlock(&lock);
    } else if(READSIZE == SOCKET_ERROR) {
        perror("Error reading data. Closing client socket");

        //Remove mutex lock
        printf("\nRemoving lock");
        pthread_mutex_unlock(&lock);

        return NULL;
    }

    // Received file name to upload from a client
    bzero(file_name, MESSAGE_SIZE);
    READSIZE = recv(client_socket, file_name, MESSAGE_SIZE, 0);

    // Check data received correctly
    if(READSIZE == 0) {
        puts("Client disconnected");
        fflush(stdout);

        //Remove mutex lock
        printf("\nRemoving lock");
        pthread_mutex_unlock(&lock);
    } else if(READSIZE == SOCKET_ERROR) {
        perror("Read error");
        write(client_socket, "File upload failed\n", strlen("File upload failed\n"));

        //Remove mutex lock
        printf("\nRemoving lock");
        pthread_mutex_unlock(&lock);

        return NULL;
    }

    // Received file path to upload from a client
    bzero(save_path, MESSAGE_SIZE);
    READSIZE = recv(client_socket, save_path, MESSAGE_SIZE, 0);

    // Check data received correctly
    if(READSIZE == 0) {
        puts("Client disconnected");
        fflush(stdout);

        printf("\nRemoving lock");
        pthread_mutex_unlock(&lock);
    } else if(READSIZE == SOCKET_ERROR) {
        perror("Read error");
        write(client_socket, "File upload failed\n", strlen("File upload failed\n"));

        //Remove mutex lock
        printf("\nRemoving lock");
        pthread_mutex_unlock(&lock);

        return NULL;
    }

    // Store file location and name
    char* fr_name = (char*) malloc(2 + strlen(fr_path) + strlen(save_path) + strlen(file_name));
    strcpy(fr_name, fr_path);
    strcat(fr_name, save_path);
    strcat(fr_name, "/");
    strcat(fr_name, file_name);
    
    pid_t pid;
    pid = fork();
    
    // Child process: Perform file creation on users behalf
    if(pid == 0) {
		struct passwd* pw;

		// Check if details were retrieved successfully
		if((pw = getpwnam(username)) == NULL) {
		    perror("Failed to retrieve user details");

            //Remove mutex lock
            printf("\nRemoving lock");
		    pthread_mutex_unlock(&lock);

		    return NULL;
		}

		// Get groups user belongs to
		int ngroups = 50;

		// Retrieve user groups
		gid_t *supp_groups;
		gid_t *groups;
		groups = (gid_t*) malloc(ngroups * sizeof(gid_t));
		supp_groups = (gid_t*) malloc(ngroups * sizeof(gid_t));

        // Retrieve UID of user
		uid_t uid = pw->pw_uid;

        // Get groups user is a part of and check they were retrieved correctly
		if(getgrouplist(pw->pw_name, uid, groups, &ngroups) == -1) {
			perror("ngroups is too small");

            //Remove mutex lock
            printf("\nRemoving lock");
		    pthread_mutex_unlock(&lock);

            return NULL;
		}

		// Get all groups associated with user
		for(int j = 0; j < ngroups; j++) {
		    supp_groups[j] = groups[j];
		}
		
		// Set gid, euid, and egid
		gid_t gid = pw->pw_gid;
		uid_t euid = uid;
		gid_t egid = gid;

        printf("Creating file on behalf of %s\n", username);
		printf("UID: %d\n", uid);
		printf("GID: %d\n", gid);

		// Change from root to user
		setgroups(ngroups, supp_groups);
		setreuid(uid, uid);
		setregid(uid, gid);
		seteuid(uid);
		setegid(uid);

		// Open file for writing
		FILE *fr = fopen(fr_name, "w");

		// Check file opened successfully
		if(fr == NULL) {
		    printf("\nFile %s Cannot be opened on server\n", fr_name);
		    write(client_socket, "File upload failed\n", strlen("File upload failed\n"));
		    close(client_socket);

            //Remove mutex lock
            printf("\nRemoving lock");
		    pthread_mutex_unlock(&lock);

		    return NULL;
		} 

		free(fr_name);

		// Receive file data from the client
		while(true) {
		    READSIZE = recv(client_socket, message, MESSAGE_SIZE, 0);

		    // Check data received correctly
		    if(READSIZE == 0) {
		        break;
		    } else if(READSIZE == SOCKET_ERROR) {
		        perror("Read error");
		        write(client_socket, "File upload failed\n", strlen("File upload failed\n"));
		        fclose(fr);
		        close(client_socket);

		        //Remove mutex lock
                printf("\nRemoving lock");
                pthread_mutex_unlock(&lock);

		        return NULL;
		    }

		    // Store data in file and check if stored correctly
		    if((fprintf(fr, "%s", message)) < 0) {
		        write(client_socket, "File upload failed\n", strlen("File upload failed\n"));
		        fclose(fr);
		        close(client_socket);

                //Remove mutex lock
                printf("\nRemoving lock");
                pthread_mutex_unlock(&lock);
		        
		        return NULL;
		    }

		    // Close file and update user on success
		    fclose(fr);
		    write(client_socket, "File uploaded successfully\n", strlen("File uploaded successfully\n"));
		    bzero(message, MESSAGE_SIZE);
		}
    } else {
        // Wait for child process to complete
    	wait(NULL);
    }

    // unlock mutex
    pthread_mutex_unlock(&lock);

    // Close client connection
    puts("\nData written successfully\nClosing connection to client");
    close(client_socket);

    return NULL;
}
