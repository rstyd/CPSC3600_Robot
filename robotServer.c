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

int command;

int main(int argc, char *argv[])
{

    signal(SIGINT, interupt);          /*Ctrl-c Signal*/

    serverPort = 8080;     /* server port */
    servIP = (char *)malloc(400);                    /* IP address of server */
    page = (char *)malloc(SENDMAX);                     /*File location on the host*/

    char *id, *imageID;
    id = argv[3];
    imageID = argv[4];
    char *imageAddr = malloc(100);
    sprintf(imageAddr, "/snapshot?topic=/robot_%s/image?width=600?height=500", imageID);



   char *action = malloc(100);//"/twist?id=";
    sprintf(action, "/twist?id=%s", id); 
    char *dGPS = malloc(100);
    sprintf(dGPS, "/state?id=%s", id);

    char *lasers = malloc(100);//8083
	sprintf(lasers, "/state?id=%s", id);
	

 servIP[0] = 0;
    servIP = argv[2];
    serverPort = atoi(argv[1]);
    host = "castara.cs.clemson.edu";

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
        requestMsg *newRequest = getRequest(resp); 

        char *page;
        char *cmd = newRequest->command;
        // Parse Commands
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
            puts("MOVE");
            command = MOVE;
            char *newPage = malloc(130);
            strcpy(newPage, action);
            strcat(newPage, "&lx=");
            strtok(cmd, " ");
            char *vel = strtok(NULL, " ");
            page = strcat(newPage, vel);
            robotAddr.sin_port = htons(8082);
        }
        else if(strstr(cmd, "TURN") != NULL){
            puts("TURN");
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
            puts("STOP");
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
        
        // Set the appropriate socket
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

        unsigned char *content = (unsigned char *) strstr((char *)response, "\r\n\r\n");
        content = content + 4;
        
        // Create the responce header structure
        responseSize -= content - response;
        struct response_message *rm = (struct response_message *) malloc(sizeof( struct response_message));
        int number = ceil(1.0 * responseSize/(1000 - sizeof(struct response_message)));
        rm->nMessages = number; 
        int sequence = 0;
        int offset = 0;

        int transmission = 1000 - sizeof(responseMsg) + sizeof(void *);
        unsigned char *buff = malloc(1000);
        rm->data = malloc(transmission);
            
        // Fragment the data into 988 blocks and send them out to the client using UDP
        while(sequence < number){
              rm->sequenceN = sequence;
             rm->nMessages = number;
            if (sequence == number - 1){
                memcpy(rm->data, content + offset, (responseSize % transmission));
                memcpy(buff, &newRequest->commID, 5); 
                memcpy(buff + 4, &rm->nMessages, 4);
                memcpy(buff + 8, &rm->sequenceN, 4);
                memcpy(buff + 12, rm->data, (responseSize % transmission));
                sendUDP(sockUDP, buff, (responseSize % transmission) + 12 );
                break;
            }
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
    int sent;
    if ((sent = sendto(sockUDP, (unsigned char *)message, size, 0, (struct sockaddr *)
                &clientAddr, sizeof(clientAddr))) != size) {
        DieWithError("ERROR\tSent wrong # of bytes");
    }
}

// Recv UDP messages from the client
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

    int sent;
    if ((sent = send(sockTCP, (char *)message, size, 0)) != size){
        DieWithError("ERROR\tSent wrong # of bytes");
    }
    
    fprintf(stderr, "Sent successfully!\n");
}

// Recieves a TCP message from the robot server
unsigned char *recvTCP(int sock){
    fprintf(stderr, "getting response from robot\n");
    if (command != IMAGE) { 
		 char buffer[20000];
		 int bytesRcvd;
		 if ((bytesRcvd = recv(sock, buffer, 20000, 0)) <= 0) {
			  DieWithError("recvFailed");
		 }

		 unsigned char *cont = malloc(bytesRcvd);
		 memcpy(cont, buffer, bytesRcvd);
		 responseSize = bytesRcvd;
		 close(sockTCP);
    	return cont;
	 }
	 else {
         // Only used for the image which doesn't come in one stream thanks to the fact that that particular server uses http/1.0 
		 int size_recv , total_size= 0;
		 char chunk[99999];
		 unsigned char *cont = malloc(20480);   //malloced 20KB

		 while(1)
		 {
			  memset(chunk ,0 , 99999);  //clear the variable
			  if((size_recv =  recv(sock , chunk , 99999 , 0) ) < 0)
			  {
					break;
			  }
			  else if(size_recv == 0){
				 fprintf(stderr,"finished receiving says recv()\n");
				 break;
			  }
			  else
			  {
					memcpy(cont + total_size , chunk ,size_recv);
					total_size += size_recv; 
			  }
			  
			 // printf("recvsize is: %d\n", size_recv);
		 }
	 	responseSize = total_size; 
		close(sockTCP);
    	return cont;
	 }
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
    unsigned int *commID = (unsigned int *) resp;

    char *robotID;
    int robotIdLen = strlen((char *) resp + 4);
    robotID = malloc(robotIdLen + 1);
    memcpy(robotID, resp + 4, robotIdLen + 1);

    char *command;
    int commandLen = strlen((char *) resp + 4 + robotIdLen + 1);
    command = malloc(commandLen + 1);
    memcpy(command, resp + 4 + robotIdLen + 1, commandLen);
    
    request->commID = *commID;

    request->robotID = malloc(robotIdLen + 1);
    strcpy(request->robotID, robotID);

    request->command = malloc(commandLen + 1);
    strcpy(request->command, command);
    return request;
}

// Attempts to resolve the host
void resolveHost() {
    char tmp[SENDMAX];

    thehost = gethostbyname(servIP);
    if(thehost == NULL){
        memcpy(tmp, servIP + 7, strlen(servIP) + 1 - 7);
        thehost = gethostbyname(tmp);
    }
    if(thehost == NULL){
        thehost = gethostbyname(host);
    }

    if(thehost == NULL){
        host = strtok(host, ":");
        if(host[0] == 'h'){
            char *more = strtok(NULL, ":");
            host = more + 2;
        }
        char *p = strtok(NULL, ":");
        if(p){
            robotAddr.sin_port = htons(atoi(p));
        }
        thehost = gethostbyname(host);
        fprintf(stderr, "Had to do bad things with host %s\n", host);
    }

    if(thehost == NULL){
        DieWithError("ERROR\tcan't resolve name");
    }

    robotAddr.sin_addr.s_addr = 
        *((unsigned long *) thehost->h_addr_list[0]);

}
