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
unsigned char *recvTCP(int sock);

struct sockaddr_in clientAddr;

int sockTCP;

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

    char *imageAddr2 = "robot_50000";//8081
    char *imageAddr = malloc(strlen(imageAddr2 + 30));
    

    char *imageAddr3 = "/snapshot?topic=/robot_5/image?width=600?height=500";//8081
    char *action3 = "/twist?id=2agreeable";//8082 move: &lx=, turn: &az=, stop: &lx=0
    char *dGPS3 = "/state?id=2agreeable";//8084
    char *lasers3 = "/state?id=2agreeable";//8083


    id = argv[3];
    imageID = argv[4];
    sprintf(imageAddr, "/snapshot?topic=/robot_%s/image?width=600?height=500", imageID);


   char *action = malloc(100);//"/twist?id=";
   char *action2 = malloc(strlen(action2) + 30); 
    sprintf(action, "/twist?id=%s", id); 
    char *dGPS2 = "/state?id=";
    sprintf(dGPS, "/state?id=%s", id);
   //char *action = malloc(strlen(action2) + 30); 

    char *dGPS = malloc(100);
    char *lasers = malloc(100);//8083
	sprintf(lasers, "/state?id=%s", id);
	
	char *GPS = malloc(100);
	sprintf(GPS, "/state?id=%s", id);

 servIP[0] = 0;
    servIP = argv[2];
    serverPort = atoi(argv[1]);
    printf("Port: %hu Ip %s ID %s imageID %s", serverPort, servIP, id, imageID);
    host = "castara.cs.clemson.edu";

    printf("THE HOST IS %s\n", host);	

    
    /*Create UDP socket*/
    if ((sockUDP = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("ERROR\tSocket Error");

    /* Construct the server address structure */
    memset(&receptionAddr, 0, sizeof(receptionAddr));/* Zero out structure */
    receptionAddr.sin_family = AF_INET;         /* Internet addr family */
    receptionAddr.sin_port   = htons(serverPort);     /* Server port */

    /*Robot Adresses*/
    memset(&robotAddr, 0, sizeof(robotAddr));
    robotAddr.sin_family = AF_INET;
    robotAddr.sin_port = htons(serverPort);
    robotAddr.sin_addr.s_addr = inet_addr(servIP);

    /*Resolving address if need be*/
    if (robotAddr.sin_addr.s_addr == -1) {
        resolveHost();
    }
    
        //printf("NUMBA%d\n", number);
    receptionAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockUDP, (struct sockaddr *) 
                &receptionAddr, sizeof(receptionAddr)) < 0)
        DieWithError("bind() failed");
   
   //Connect TCP Sockets// 
  
    
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
            char *newPage = malloc(100);
            page = strcpy(newPage, imageAddr);
            robotAddr.sin_port = htons(8081);
        }
        else if(strstr(cmd, "DGPS") != NULL){
            command = DGPS;
            puts("DGPS");
            char *newPage = malloc(70);
            page = strcpy(newPage, dGPS);
            robotAddr.sin_port = htons(8084);
        }else if(strstr(cmd, "GPS") != NULL){
            command = GPS;
            puts("GPS");
            char *newPage = malloc(80);
            page = strcpy(newPage, dGPS);
            robotAddr.sin_port = htons(8082);
        }
        else if(strstr(cmd, "LASERS") != NULL){
            command = LASERS;
            puts("LASERS");
            char *newPage = malloc(30);
            page = strcpy(newPage, lasers); 
            robotAddr.sin_port = htons(8083);
        }
        else if(strstr(cmd, "MOVE") != NULL){
            command = MOVE;
            char *newPage = malloc(130);
            strcpy(newPage, action);
            strcat(newPage, "&lx=");
            strtok(cmd, " ");
            char *vel = strtok(NULL, " ");
            page = strcat(newPage, vel);
            //printf("MOVE %s\n", page);
            robotAddr.sin_port = htons(8082);
        }
        else if(strstr(cmd, "TURN") != NULL){
            command = TURN;
            char *newPage = malloc(130);
            strcpy(newPage, action);
            strcat(newPage, "&az=");
            strtok(cmd, " ");
            char *vel = strtok(NULL, " ");
            page = strcat(newPage, vel);
            robotAddr.sin_port = htons(8082);
        }
        else if(strstr(cmd, "STOP") != NULL){
            command = STOP;
            char *newPage = malloc(130);
            strcpy(newPage, action);
            page = strcat(page, "&lx=0");
            robotAddr.sin_port = htons(8082);
        }

        char *query = malloc(sizeof(char) * 500);
        sprintf(query, 
                "GET %s HTTP/1.0\r\n"
                "User-Agent: wget/1.14 (linux-gnu)\r\n"
                "Host: %s\r\n"
                "Connection: Keep-Alive\r\n"
                "\r\n", page, host);

        unsigned char *response;
        if (command == STOP || command == TURN || command == MOVE || command == GPS) {
            sendTCP(sockTCP, (unsigned char *)query, strlen(query));
            response =  recvTCP(sockTCP);
        }
        else if (command == LASERS) {
            sendTCP(sockTCP, (unsigned char *)query, strlen(query));
            response =  recvTCP(sockTCP);

        }
        else if (command == DGPS) {
            sendTCP(sockTCP, (unsigned char *)query, strlen(query));
            response =  recvTCP(sockTCP);

        }
        else if (command == IMAGE) {
            sendTCP(sockTCP, (unsigned char *)query, strlen(query));
            response =  recvTCP(sockTCP);
        }


        puts("HMM"); 
        //close(sockTCP);
        printf("%s", response);
        //puts("GOT TCP STUFF");

        unsigned char *content = strstr((char *)response, "\r\n\r\n");
        if(content == NULL){
            //fprintf(stderr, "Issue with content request\n%s", content);
        }
        content = content + 4;
        
        if (command == IMAGE) {
        } else {

        }
        printf("ORIGINAL RESPONSE: %d\n", responseSize);
        responseSize -= content - response; 
        //printf("Content: %s\n", content);
        struct response_message *rm = (struct response_message *) malloc(sizeof( struct response_message));
        unsigned char *data;
        int number = ceil(1.0 * responseSize/(1000 - sizeof(struct response_message)));
        rm->nMessages = number; 
        int sequence = 0;
        int offset = 0;

        printf("Requires %d messages\n", number);
        printf("Response Size: %d\n", responseSize);

        int transmission = 1000 - sizeof(responseMsg) + sizeof(void *);
        unsigned char *buff = malloc(1000);
        rm->data = malloc(transmission);

        while(sequence < number){
              rm->sequenceN = sequence;
             rm->nMessages = number;
            int fSize = transmission - responseSize;
            if (sequence == number - 1){
                memcpy(rm->data, content + offset, (responseSize % transmission));
                memcpy(buff, &newRequest->commID, 5); 
                memcpy(buff + 4, &rm->nMessages, 4);
                memcpy(buff + 8, &rm->sequenceN, 4);
                memcpy(buff + 12, rm->data, (responseSize % transmission));
                //memcpy(rm->data, content + offset, (responseSize % transmission));
               // memcpy(buff + 12, content + offset, (responseSize % transmission) + 12);
                sendUDP(sockUDP, buff, (responseSize % transmission) + 12 );
                break;
            }
           // memcpy(buff + 12, content + offset, transmission);
            memcpy(rm->data, content + offset, (transmission));
            offset += transmission;
            memcpy(buff, &newRequest->commID, 4); 
            memcpy(buff + 4, &rm->nMessages, 4);
            memcpy(buff + 8, &rm->sequenceN, 4);
            memcpy(buff + 12, rm->data, transmission);
            sendUDP(sockUDP, buff, 1000);
            sequence++;
            memset(buff, 0, 1000);
        }//end sequence loop
    }//end ETERNAL LOOP
    return 0;
}//end main


// Sends a UDP message through the specified socket
void sendUDP(int sock, unsigned char *message, int size){
    fprintf(stderr, "Sending response to client\n");
    int sent;
    printf("SENDING SIZE: %d\n", size);
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
    /* Create TCP sockets */
    if ((sockTCP = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        DieWithError("ERROR\tSocket Error");

    if(connect(sockTCP, (struct sockaddr *)
                &robotAddr, sizeof(robotAddr)) < 0){
        DieWithError("ERROR\tUnable to connect");
         }

    fprintf(stderr, "Sending\n");
    printf("%s\n", message);
    printf("Size sent TCP %d\n", size);

    int sent;
    if ((sent = send(sockTCP, (char *)message, size, 0)) != size){
        printf("SENT: %d\n", sent);
        DieWithError("ERROR\tSent wrong # of bytes");
    }
    
    fprintf(stderr, "Sent successfully!\n");
}

// Recieves a TCP message from the robot server
unsigned char *recvTCP(int sock){
    fprintf(stderr, "getting response from robot\n");
    char buffer[20000];
    int bytesRcvd;

   
    while (1) {
        if ((bytesRcvd = recv(sock, buffer, 20000, 0)) <= 0) {
            DieWithError("recvFailed");
        }
    }
    //printf("GOT %d\n", bytesRcvd);
    unsigned char *cont = malloc(bytesRcvd);
    memcpy(cont, buffer, bytesRcvd);
    //printf("%s\n", buffer); 
    responseSize = bytesRcvd;
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
    unsigned int *commID = (unsigned int *) resp;

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
    
    request->commID = *commID;

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
