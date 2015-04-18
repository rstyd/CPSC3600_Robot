#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <netdb.h>      /* for getHostByName() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <stdbool.h>

#define RCVBUFSIZE 32   /* Size of receive buffer */

void DieWithError(char *errorMsg) {
    fprintf(stderr, "Error: %s\n", errorMsg);
    exit(1);
}

int main(int argc, char *argv[])
{
    // If we don't at least have the url print the usage statement and exit
    if (argc < 2) {
        printf("Usage: simget URL [-p serverPort] [-O filename]\n");
        exit(1);
    }

    int sock;
    struct sockaddr_in serverAddr;

    char *hostname; // The requested URL. Required
     unsigned short serverPort = 80; // The web server's port. Default
    char *filename = ""; // Save the document in this file
    char document[500]; // The document on the server the user wants to get
    bool saveToFile = false; // Check whether the file has been specified
    char *doc = NULL;
    hostname = argv[1]; // Set the hostname
    // Find the item that we are trying to get
    // www.reddit.com/r/hiphopheads 
    // If the url does not contain an http
    if (strstr(hostname, "http") != NULL)
        hostname = hostname + 7;
        doc = strchr(hostname, '/'); 
       // printf("DOC %s", doc);
        if (doc != NULL) {
            strcpy(document, doc);
            hostname[doc - hostname] = '\0';  
        }
        else {
            strcpy(document, "/");
        }

    // Initialize the socket
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        DieWithError("socket() failed");
    }

     char c;
    argv = argv + 1;
    argc = argc - 1;
    // Parse the optional arguments
    while ((c = getopt(argc, argv, "p:O:")) != -1) {
        switch(c) {
            case 'p':
                printf("Port: %hu\n", serverPort);
               serverPort =  atoi(optarg);
                break;
            case 'O':
                saveToFile = true;
                filename = optarg;
                break;
        }
    }

    // Set up the address structure
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
   // serverAddr.sin_addr.s_addr = inet_addr(hostname);
    serverAddr.sin_port = htons(serverPort);
      
    // Get the IP address of the host
    printf("%s", hostname);
    struct hostent *thehost = gethostbyname(hostname);
    if (thehost == NULL)
        DieWithError("Could not get IP address of host");
    // Give the address structure the IP address
    serverAddr.sin_addr.s_addr = *((unsigned long *) thehost->h_addr_list[0]);

       
    char *host = strstr(hostname, "/");
    printf("Hostname: %s\n", hostname);

    char header[3000]; 
    char request[50000];
    sprintf(header,"GET %s HTTP/1.1\r\n", document);
    sprintf(request, "%sHost: %s\r\n\r\n", header, hostname);

    printf("Request: %s", request);
    int requestLen = sizeof(request);

    if (connect(sock, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0)
        DieWithError("connect() failed");
    puts("CONNECTED");
    if (send(sock, request, requestLen, 0) != requestLen)
        DieWithError("send() sent a different number of bytes than expected");
    char responseBuffer[50000];
    memset(responseBuffer, 0, 50000);
    int bytesRcvd;

    if ((bytesRcvd = recv(sock, responseBuffer, 50000, 0)) <= 0) {
        DieWithError("recvFailed");
    }
    char *buf = strstr(responseBuffer, "\r\n\r\n");
    printf("Response: %s\n", responseBuffer);
    if (saveToFile == true) {
        FILE *fp = fopen(filename, "w");
        fprintf(fp,"%s", buf);
        fclose(fp);
    }

    return 0;
}
