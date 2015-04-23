/*Sean Southard, smsouthExam2.c*/

#include <netdb.h>      /* for getHostByName() */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>     /* for memset() */
#include <netinet/in.h> /* for in_addr */
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <unistd.h>     /* for close() */
#include <signal.h>     /*to use the ctrl-c interupt*/
#include <sys/time.h>//Used keep track of execution time
#include "robots.h"
#include <math.h>
#include <stdbool.h>

#define SENDMAX 50000     /* Longest string to recieve, 
                             also used to init arrays */

extern int errno;

//executable_name server_port IP/host_name ID image_ID
void DieWithError(char *errorMessage);  /* External error handling function */
void interupt(int sig);

struct sockaddr_in receptionAddr; /* server address */
struct sockaddr_in robotAddr;     /* Source address */

requestMsg *getRequest(unsigned char *resp); 
void resolveHost();
void sendUDP(int sock, unsigned char *message, int size);
unsigned char *recvUDP(int sock, struct sockaddr_in serverAddr);

void sendTCP(int sock, unsigned char *message, int size);
unsigned char *recvTCP();

struct sockaddr_in clientAddr;

int sockTCP;                        /* Socket descriptor */
int sockUDP;

int responseSize;	

struct hostent *thehost;         /* Hostent from gethostbyname() */

char *servIP;
char *page; 
char *host;
unsigned short serverPort; 

int main(int argc, char *argv[])
{

    signal(SIGINT, interupt);          /*Ctrl-c Signal*/

    serverPort = 8080;     /* server port */
    servIP = (char *)malloc(400);                    /* IP address of server */
    page = (char *)malloc(SENDMAX);                     /*File location on the host*/

    char *id, *imageID;
    //	char *tok; //, *host = (char *)malloc(SENDMAX);

    id = argv[3];
    imageID = argv[4];

    servIP[0] = 0;
    servIP = argv[2];
    serverPort = atoi(argv[1]);
    printf("Port: %hu Ip %s ID %s imageID %s", serverPort, servIP, id, imageID);
    host = "castara.cs.clemson.edu";

    printf("THE HOST IS %s\n", host);	

    /* Create a TCP socket */
    if ((sockTCP = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        DieWithError("ERROR\tSocket Error");

    if ((sockUDP = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("ERROR\tSocket Error");

    /* Construct the server address structure */
    memset(&receptionAddr, 0, sizeof(receptionAddr));/* Zero out structure */
    receptionAddr.sin_family = AF_INET;         /* Internet addr family */
    receptionAddr.sin_port   = htons(serverPort);     /* Server port */

    memset(&robotAddr, 0, sizeof(robotAddr));
    robotAddr.sin_family = AF_INET;
    robotAddr.sin_port = htons(serverPort);

    /*Server IP address*/
    robotAddr.sin_addr.s_addr = inet_addr(servIP);


    /*Resolving address if need be*/
    if (robotAddr.sin_addr.s_addr == -1) {
        resolveHost();
    }
    
    receptionAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockUDP, (struct sockaddr *) 
                &receptionAddr, sizeof(receptionAddr)) < 0)
        DieWithError("bind() failed");

    while(true){
        unsigned char *resp = recvUDP(sockUDP, receptionAddr);
        puts("MAKING request");
        requestMsg *newRequest = getRequest(resp); 

        printf("Robot ID: %s Command: %s\n", newRequest->robotID, newRequest->command);

        //TODO: set page according to resp
        //char *page = dGPS;

        char *page;
        char *cmd = newRequest->command;
        int command;
        printf("COMMAND IS %s\n", cmd);
        if(strstr(cmd, "IMAGE") != NULL){
            command = IMAGE;
            page = imageAddr;
            robotAddr.sin_port = htons(8081);
        }
        if(strstr(cmd, "DGPS") != NULL){
            command = DGPS;
            puts("DGPS");
            page = dGPS;
            robotAddr.sin_port = htons(8084);
        }else if(strstr(cmd, "GPS") != NULL){
            command = GPS;
            puts("GPS");
            page = action;
            robotAddr.sin_port = htons(8082);
        }
        if(strstr(cmd, "LASERS") != NULL){
            command = LASERS;
            puts("LASERS");
            page = lasers;
            robotAddr.sin_port = htons(8083);
        }
        if(strstr(cmd, "MOVE") != NULL){
            command = MOVE;
            page = action;
            page = strcat(page, "&lx=");
            strtok(cmd, " ");
            char *vel = strtok(NULL, " ");
            page = strcat(page, vel);
            robotAddr.sin_port = htons(8082);
        }
        if(strstr(cmd, "TURN") != NULL){
            command = TURN;
            page = action;
            page = strcat(page, "&az=");
            strtok(cmd, " ");
            char *vel = strtok(NULL, " ");
            page = strcat(page, vel);
            robotAddr.sin_port = htons(8082);
        }
        if(strstr(cmd, "STOP") != NULL){
            command = STOP;
            page = action;
            page = strcat(page, "&lx=0");
            robotAddr.sin_port = htons(8082);
        }

        char *query = malloc(sizeof(char) * 500);
        sprintf(query, 
                "GET %s HTTP/1.1\r\n"
                "User-Agent: wget/1.14 (linux-gnu)\r\n"
                "Host: %s\r\n"
                "Connection: Keep-Alive\r\n"
                "\r\n", page, host);

        sendTCP(sockTCP, (unsigned char *)query, strlen(query));
        puts("HMM"); 
        unsigned char *response =  recvTCP(sockTCP, robotAddr);
        printf("%s", response);
        responseSize = strlen(response);
        puts("GOT TCP STUFF");

        unsigned char *content = strstr((char *)response, "\r\n\r\n");
        int contentSize = content - response - 4;
        if(content == NULL){
            //fprintf(stderr, "Issue with content request\n%s", content);
        }
        content = content + 4;
        printf("Content: %s\n", content);
        struct response_message *rm = (struct response_message *) malloc(sizeof( struct response_message));
        unsigned char *data;
        int number = ceil(1.0 * responseSize/(1000 - sizeof(struct response_message)));
        rm->nMessages = number; 
        int sequence = 0;
        int offset = 0;

        printf("Requires %d messages\n", number);
        printf("Size %zu\n", 1000 - sizeof(responseMsg) + sizeof(void * ));
        printf("Response Size: %d\n", responseSize);
        if (command == MOVE || command == TURN || command == STOP) {
            // Just send the header
        }

        int transmission = 1000 - sizeof(responseMsg) + sizeof(void *);
        while(sequence < number){
            rm->sequenceN = sequence;
            if (sequence == number - 1){
                rm->data = content + (responseSize % transmission);
                memcpy(rm->data, content + offset, responseSize % transmission);
                sendUDP(sockUDP, (unsigned char *)rm, (responseSize % transmission));
                break;
            }
            
            memcpy(rm->data, content + offset, transmission);
            rm->data = content + offset;
            offset += transmission;
            sendUDP(sockUDP, (unsigned char *) rm, 1000);
        }//end sequence loop
    }//end ETERNAL LOOP
    return 0;
}//end main


// Sends a UDP message through the specified socket
void sendUDP(int sock, unsigned char *message, int size){
    fprintf(stderr, "Sending response to client\n");
    int sent;
    if ((sent = sendto(sockUDP, (unsigned char *)message, size, 0, (struct sockaddr *)
                &clientAddr, sizeof(clientAddr))) != size) {
        printf("SENT: %d\n", sent);
        DieWithError("ERROR\tSent wrong # of bytes");
    }
}

unsigned char *recvUDP(int sock, struct sockaddr_in allAddress){

    fprintf(stderr,"waiting for command\n");
    char buffer[SENDMAX];
    int respStringLen;
        unsigned int clntLen = sizeof(clientAddr);

    if ((respStringLen = recvfrom(sockUDP, buffer, SENDMAX, 0, 
                    (struct sockaddr *) &clientAddr, &clntLen)) < 0){
        DieWithError("ERROR\tRecieve Error");
    }
    unsigned char *cont = malloc(respStringLen);
    memcpy(cont, buffer, respStringLen );
    return cont;
}

// Sends a TCP message of size bytes
void sendTCP(int sock, unsigned char *message, int size){ 
    fprintf(stderr, "Connecting to robot\n");
    if(connect(sockTCP, (struct sockaddr *)
                &robotAddr, sizeof(robotAddr)) < 0){
        DieWithError("ERROR\tUnable to connect");
    }
    puts("SDDS");
    fprintf(stderr, "Sending\n");
    printf("%s\n", message);
    if ((send(sock, (char *)message, size, 0)) != size){
        DieWithError("ERROR\tSent wrong # of bytes");
    }
    
    fprintf(stderr, "Sent successfully!\n");
}

// Recieves a TCP message from the robot server
unsigned char *recvTCP(){
    fprintf(stderr, "getting response from robot\n");
    char buffer[5000];
    int bytesRcvd;
    if ((bytesRcvd = recv(sockTCP, buffer, 5000, 0)) <= 0) {
        DieWithError("recvFailed");
    }

    //printf("GOT %d\n", bytesRcvd);
    unsigned char *cont = malloc(bytesRcvd);
    memcpy(cont, buffer, bytesRcvd);
    //printf("%s\n", buffer); 
    responseSize = bytesRcvd;
    return cont; 

    close(sockTCP); 
    return cont;            	
}

void DieWithError(char *errorMessage)
{
    fprintf(stderr, "%s\n", errorMessage);
    exit(1);
}

//Exits on ctrl-c
void interupt(int sig){
    fprintf(stderr, "\nEnding it all!\n");
    close(sockTCP);
    exit(0);
}

// Parses the request into a proper request object
requestMsg *getRequest(unsigned char *resp) {
    requestMsg *request = malloc(sizeof(requestMsg));
    printf("%s", resp + 4);
    unsigned int commID = resp[0] * pow(10, 3) +  resp[1] * pow(10, 2) + resp[2] * pow(10, 1) + resp[3];

    char *robotID;
    int robotIdLen = strlen(resp + 4);
    robotID = malloc(robotIdLen + 1);
    memcpy(robotID, resp + 4, robotIdLen + 1);
    printf("ROBOT ID: %s\n", robotID);

    char *command;
    int commandLen = strlen(resp + 4 + robotIdLen + 1);
    command = malloc(commandLen + 1);
    memcpy(command, resp + 4 + robotIdLen + 1, commandLen);
    printf("COMMAND %s\n", command);
    
    request->commID = commID;

    request->robotID = malloc(robotIdLen + 1);
    strcpy(request->robotID, robotID);

    request->command = malloc(commandLen + 1);
    strcpy(request->command, command);
    return request;
}

void resolveHost() {
    char tmp[SENDMAX];

    //fprintf(stderr, "Resolving address '%s'\n", servIP);
    thehost = gethostbyname(servIP);
    if(thehost == NULL){
        memcpy(tmp, servIP + 7, strlen(servIP) + 1 - 7);
        thehost = gethostbyname(tmp);

        /*fprintf(stderr, 
          "	Resolve failed, trying %s\n",tmp);*/
    }
    if(thehost == NULL){
        thehost = gethostbyname(host);
        /*fprintf(stderr, 
          "	Resolve failed again, trying %s\n",host);*/
    }

    if(thehost == NULL){
        host = strtok(host, ":");
        if(host[0] == 'h'){
            char *more = strtok(NULL, ":");
            //strcat(host, more);
            host = more+2;
        }
        char *p = strtok(NULL, ":");
        if(p){
            robotAddr.sin_port = htons(atoi(p));
        }
        thehost = gethostbyname(host);
        fprintf(stderr, "Had to do bad things with host %s\n", host);
    }

    if(thehost == NULL){
        //fprintf(stderr, "\n%s\n", servIP);
        DieWithError("ERROR\tcan't resolve name");
    }

    robotAddr.sin_addr.s_addr = 
        *((unsigned long *) thehost->h_addr_list[0]);

}
