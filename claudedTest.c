#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <netdb.h>      /* for getHostByName() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <stdbool.h>
#include <signal.h>

#define RCVBUFSIZE 32   /* Size of receive buffer */
#define MAXPENDING 1

void DieWithError(char *errorMsg) {
    fprintf(stderr, "Error: %s\n", errorMsg);
    exit(1);
}

int HandleTCPClient(int clntSocket, char *fileBuffer, char *name, char *extension, int *nfiles);
void ctrlHandler();

int main(int argc, char *argv[])
{
    // If we don't at least have the url print the usage statement and exit
    if (argc < 2) {
        printf("Usage for clientMode: testProject 0 [serverName] [serverPort] filePathToTransfer\n");
        printf("Usage for serverMode: testProject 1 [serverPort] filePathToCreate\n");
        exit(1);
    }
    
    int clientServerFlag = atoi(argv[1]);
    char *serverName;
    unsigned int serverPort;
    char *filePath;
     
    // Address structs 
    struct sockaddr_in serverAddr;
    struct sockaddr_in clientAddr;
    // Sockets
    int clientSock, serverSock;
    struct hostent *thehost;      
    // The number of files sent to the server
    int files;
    
    signal(SIGINT, ctrlHandler); 

    if (clientServerFlag == 0) {
        serverName = argv[2];
        serverPort = atoi(argv[3]);
        filePath = argv[4];

        // Initialize the socket
        if ((clientSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
            DieWithError("socket() failed");
        }

        // Set up the address structure
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = inet_addr(serverName);
        serverAddr.sin_port = htons(serverPort);

        /* If user gave a dotted decimal address, we need to resolve it  */
        if (serverAddr.sin_addr.s_addr == -1) {
            thehost = gethostbyname(serverName);
            serverAddr.sin_addr.s_addr = *((unsigned long *) thehost->h_addr_list[0]);
        }

       // Connect to the server 
        if (connect(clientSock, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0)
            DieWithError("connect() failed");
       

        FILE *file = fopen(filePath, "rb");
        fseek(file, 0, SEEK_END);
        // Get the size of the file
        size_t fileSize = ftell(file);
        // Go back to the top of the file
        rewind(file);
        printf("Size %zu\n", fileSize); 
        char sizeBuffer[20];
        sprintf(sizeBuffer, "%zu", fileSize);
        
        // Send the file to the server
        if (send(clientSock, sizeBuffer, 20, 0) != 20)
            DieWithError("send() sent a different number of bytes than expected");

        char *fileBuffer = malloc(fileSize); 
        // Make sure we read the right amount of data
        size_t result = fread(fileBuffer, sizeof(int), fileSize, file);
        char ackBuffer[20];


        /* Receive size message from client */
        if ((recv(clientSock, ackBuffer, 3, 0)) < 0){
            puts("COULD NOT GET ACK");
            return -1;
        }

        // If we didn't read the entire file
    //    if (result != fileSize) {
     //       printf("Error Reading File\n");
       // }
        
        // Send the fie to the server
        if (send(clientSock, fileBuffer, fileSize, 0) != fileSize)
            DieWithError("send() sent a different number of bytes than expected");
        
    }
    else {
        serverPort = atoi(argv[2]);
        printf("Port %d\n", serverPort);
        filePath = argv[3];
        printf("%s\n", filePath);
        // Find the extension 
        char *extension = strrchr(filePath, '.');
        if (extension == NULL) {
            DieWithError("Must have file extension");
        }
        char *name = malloc(strlen(filePath) + 1);
        strncpy(name, filePath, extension - filePath);
        printf("Extension is %s\n", extension);
        printf("Filename is %s\n", name);
        files = 0;
        /* Create socket for incoming connections */
        if ((serverSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
            DieWithError("socket() failed");

        /* Construct local address structure */
        memset(&serverAddr, 0, sizeof(serverAddr));   /* Zero out structure */
        serverAddr.sin_family = AF_INET;                /* Internet address family */
        serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
        serverAddr.sin_port = htons(serverPort);      /* Local port */

        /* Bind to the local address */
        if (bind(serverSock, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0)
            DieWithError("bind() failed");

        /* Mark the socket so it will listen for incoming connections */
        if (listen(serverSock, MAXPENDING) < 0)
            DieWithError("listen() failed");
        
        char *fileBuffer;
        int nFiles = 0;
        for (;;) /* Run forever */
        {
            /* Set the size of the in-out parameter */
            int clientLen = sizeof(clientAddr);

            /* Wait for a client to connect */
            if ((clientSock = accept(serverSock, (struct sockaddr *) &clientAddr, 
                            &clientLen)) < 0) {
                // We couldn't do anything with that client so try again
                printf("Couldn't accept client. Going to next client.\n");
                continue;
            }

            /* clntSock is connected to a client! */
            printf("Handling client %s\n", inet_ntoa(clientAddr.sin_addr));
            if (HandleTCPClient(clientSock, fileBuffer, name, extension, &nFiles) == -1) {
                continue;
            }
        }

    }
}

    
// Recieves the file from the client
int HandleTCPClient(int clntSocket, char *fileBuffer, char *name, char *extension, int *nFiles)
{
    unsigned long fileSize;  
    char sizeBuffer[50]; 
    if ((recv(clntSocket, sizeBuffer, 50, 0)) < 0){
        printf("Could not get file size.\n");
        return -1;
    }
    fileSize = strtoul(sizeBuffer, NULL, 10);

    fileBuffer = malloc(fileSize + 1);
    printf("The file is %ld bytes\n", fileSize);
    
    // send OK
    char *ack = "OK";
    if (send(clntSocket, ack, 3, 0) != 3)
        DieWithError("send() sent a different number of bytes than expected");
    /* Receive size message from client */
    if ((recv(clntSocket, fileBuffer, fileSize, 0)) < 0) {
        return -1;
    }
    fileBuffer[fileSize] = '\0';


    char *fileName = malloc(strlen(name) + strlen(extension) + 1);
    // Place name.extension into the buffer
    sprintf(fileName, "%s%d%s", name, *nFiles, extension);
    (*nFiles)++;

    FILE *file = fopen(fileName, "wb");
    fwrite(fileBuffer, sizeof(int), fileSize/sizeof(int), file);
   // fprintf(file, "%s", fileBuffer); 
    fclose(file);

    close(clntSocket);    /* Close client socket */

    return 0;
}

void ctrlHandler() {
    printf("\nCtrl-c pressed. Closing server.\n");
    exit(0);
}
