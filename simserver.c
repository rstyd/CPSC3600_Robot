#include <stdio.h>
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <sys/stat.h>
#include <time.h>
#include <errno.h>

void DieWithError(char *errMsg) {
    fprintf(stderr, "Error: %s\n", errMsg);
    exit(1);
}

void HandleTCPClient(int clntSocket);

#define RCVBUFSIZE 320
#define MAXPENDING 5    /* Maximum outstanding connection requests */

char *directory;
int main(int argc, char *argv[])
{

    int serverSock;
    int clientSock;

    struct sockaddr_in serverAddr;
    struct sockaddr_in clientAddr;

    unsigned short serverPort = 80;
    unsigned int clientLen;

    directory = "./";
    if (argc < 2) 
    {
        printf("%d\n", argc);
        fprintf(stderr, "USAGE:");
        exit(1);
    }

    // Parse the optional arguments
    char c;
    while ((c = getopt(argc, argv, "p:")) != -1) {
        switch(c) {
            case 'p':
                serverPort = atoi(optarg);
                printf("Port%hu\n", serverPort);
                break;
        }
    }
    if (optind < argc) {
        puts("HAVE A DIRECTOY"); 
        printf("%s\n", argv[optind]);
        directory = argv[optind];
    } 
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

    for (;;) /* Run forever */
    {
        /* Set the size of the in-out parameter */
        clientLen = sizeof(clientAddr);

        /* Wait for a client to connect */
        if ((clientSock = accept(serverSock, (struct sockaddr *) &clientAddr, 
                        &clientLen)) < 0)
            DieWithError("accept() failed");

        /* clntSock is connected to a client! */
        printf("Handling client %s\n", inet_ntoa(clientAddr.sin_addr));

        HandleTCPClient(clientSock);
    }

    return 0;
}

void HandleTCPClient(int clntSocket)
{
    char requestBuffer[RCVBUFSIZE];        /* Buffer for echo string */
    int reqMsgSize;                    /* Size of received message */
    char responseBuffer[RCVBUFSIZE];        /* Buffer for echo string */
    char responseCopy[RCVBUFSIZE];
    int respMsgSize;                    /* Size of received message */

    char messageBuffer[10000];

    /* Receive message from client */
    if ((reqMsgSize = recv(clntSocket, requestBuffer, RCVBUFSIZE, 0)) < 0)
        DieWithError("recv() failed");
    char *orig_req = malloc(strlen(requestBuffer) + 1);
    strcpy(orig_req, requestBuffer);
    char *method = strtok(requestBuffer, " \r\n ");
    char *resource = strtok(NULL, " \r\n");
    char *msgBuffer;

    // Used for the size of the file
    long size;

    char *last_modified;
    char *fileType;
    // Assume the response is OK until proven otherwise
    int response = 200; 
    // If we don't have the right headers
    printf("%s\n", orig_req);
    if (strcmp(method, "GET") != 0 && strcmp(method, "HEAD") != 0) {
        response = 405; 
    }
    else if (strstr(resource, "..") != NULL) {
        response = 403;
    }
    else if (strstr(orig_req, "Host: ") == NULL) {
        response = 400; 
    }
    // If they requested the file and not just the whole directory
    else if (strcmp(resource, "/") != 0) {
        puts("DSS");
        char *filename = resource + 1; 
        char *path = malloc(strlen(filename) + strlen(directory) + 2);
        strcat(path, directory); 
        if (directory[strlen(directory) - 1] != '/') {
            strcat(path, "/");
            strcat(path, filename);
        }
        printf("path ot thing %s\n", path);
        FILE *file = fopen(path, "r");
        // If the file was successfully opened
        if (file != NULL) {
            fileType = strrchr(filename, '.') + 1;
            printf("FIletype: %s", fileType);
            // Go to the end of the file
            fseek(file, 0, SEEK_END);
            // Get the size of the file
            size = ftell(file);
            // Go back to the top of the file
            rewind(file);

            // Allocate the right amount of data
            msgBuffer = (char *) malloc (sizeof(char) * size);
            // Make sure we read the right amount of data
            size_t result = fread(msgBuffer, 1, size, file);

            if (result != size) {
                printf("Error Reading File\n");
            }
        }
        else if (errno == EACCES) {
            response = 403;
        }
        else {
            response = 404;
        } 
    } 



    // Get the date for the server
    time_t t = time(NULL);
    struct tm tme = *localtime(&t);
    char date[45];
    char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    // FIgure out the date information
    char dateInfo[40];
    sprintf(dateInfo, "%d %s %d %d:%d", tme.tm_mday, months[tme.tm_mon], tme.tm_year + 1900, tme.tm_hour, tme.tm_min);
    sprintf(date, "Date: %s\r\n", dateInfo);


    // Print the information for the transaction
    fprintf(stdout, "%s\t%s\t%s\t%d\n", method, resource,dateInfo, response);


    char *server = "Server: MyHTTPD/2000\r\n";
    char *header;
    // should be replaced with malloc
    int content_length = size;
    char *finalMessage;
    int len;
    if (response == 200) {
        header  = "HTTP/1.1 200 OK\r\nConnection: close\r\n";
        char contentLength[25];
        char *contentType;
        sprintf(contentLength, "Content-Length: %ld\r\n\r\n", size);

        if (strcmp(fileType, "html") == 0)
            contentType = "Content-Type: text/html\r\n"; 
        else if (strcmp(fileType, "css") == 0)
            contentType = "Content-Type: text/css\r\n"; 
        else if (strcmp(fileType, "js") == 0)
            contentType = "Content-Type: application/javascript\r\n"; 
        else if (strcmp(fileType, "txt") == 0)
            contentType = "Content-Type: text/plain\r\n"; 
        else if (strcmp(fileType, "jpeg") == 0 || strcmp(fileType, "jpg") == 0)
            contentType = "Content-Type: image/jpeg\r\n"; 
        else if (strcmp(fileType, "pdf") == 0)
            contentType = "Content-Type: application/pdf\r\n"; 
        else 
            contentType = "Content-Type: application/octet-stream\r\n"; 
        len = strlen(header) + strlen(date) +  strlen(server) + strlen(contentLength) + strlen(contentType) + strlen(msgBuffer) + 1;
        finalMessage =  malloc(len);
        strcpy(finalMessage, header);
        strcat(finalMessage, server);
        strcat(finalMessage, date);
        strcat(finalMessage, contentType);
        strcat(finalMessage, contentLength);
        strcat(finalMessage, msgBuffer);
        finalMessage[len - 1] = '\r';
        finalMessage[len - 1] = '\n';
    }
    else if (response == 405) {
        header =  "HTTP/1.1 405 Method Not Allowed\r\nConnection: close\r\n\r\n";
        len = strlen(header) + 1;
        finalMessage = malloc(len);
        strcpy(finalMessage, header);
        finalMessage[len - 1] = '\n';
    }        
    else if(response == 404) {
        header =  "HTTP/1.1 404 Not Found\r\nConnection: close\r\n\r\n";
        len = strlen(header) + 1;
        finalMessage = malloc(len);
        strcpy(finalMessage, header);
        finalMessage[len - 1] = '\n';
    }
    else if(response == 403) {
        header =  "HTTP/1.1 403 Forbidden\r\nConnection: close\r\n\r\n";
        len = strlen(header) + 1;
        finalMessage = malloc(len);
        strcpy(finalMessage, header);
        finalMessage[len - 1] = '\n';
    }

    else if (response == 400) {

        header =  "HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\n";
        len = strlen(header) + 1;
        finalMessage = malloc(len);
        strcpy(finalMessage, header);
        finalMessage[len - 1] = '\n';

    }
    // Send response to client
    if (send(clntSocket, finalMessage, len, 0) != len)
        DieWithError("send() failed");

    close(clntSocket);    /* Close client socket */
}

