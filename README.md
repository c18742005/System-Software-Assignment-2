# System Software Assignment 2
## What the CTO wants
The CTO has offered a list of desired functionality for the updated reporting management model: 
1. No direct access to the Server folders.
2. Multiple users must be able to transfer files at the same time.
3. Files transferred must be attributed to the transfer user.
4. A specific user will have access to either the (Manufacturing/Distribution) folders. 
5. If synchronisation is an issue use a C Mutex Lock for Linux Thread Synchronization.

## Project Requirements
a. Create a Server Socket program to run on the main Server.
b. Create a Client program to connect to the server socket program.
c. The system must be capable of handling multiple clients and transfers simultaneously.
d. Transfer restrictions should be controlled using Real and Effective ID’s functionality.
e. Create 3 user accounts and link these to three groups (Manufacturing / Distribution)
f. The files transferred should be attributed to the transfer user. The file transferred to the server should show the transfer owner as its
owner.
g. The client must take a file name and path via console and transfer this to the server to be stored. The following directories are where
files can be transferred to:
    - Manufacturing
    - Distribution
These folders will be associated with the groups described in requirement (e).
h. The server must inform the client if the transfer was successful or not.

## General Assumptions
1. The solution for the problem stated above can be developed on a single machine.
2. The client will only transfer one file at a time.

## How to Run 
1. Open /src/server.c and change the fr_path variable on line 108 to the path where the distribution and manufacturing directories are located on your machine (i.e. In this directory in /server-files/)
2. Open /src/client.c and change the file_path variable on line 27 to the path where the files to be uploaded to the server are stored on your machine (i.e. In this directory in /upload/)
3. Add any files you wish to upload to the server into the uplaod directory
4. Open the /src/ directory and run `make server` and `make client` to make the required files
5. Open the /src/ directory in a terminal as root and run `./server` to start the server and await connections
6. Open the /src/ directory in a terminal and run `./client filename.ext relative_path` to send the file to the server and upload it i.e. `./client example.txt Manufacturing` to upload the example.txt file to the Manufacturing directory
7. The server will then attempt to store the file and will inform the client of the success or failure