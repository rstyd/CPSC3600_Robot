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

#define SENDMAX 50000     /* Longest string to recieve, 
							also used to init arrays */

extern int errno;

//executable_name server_port IP/host_name ID image_ID
void DieWithError(char *errorMessage);  /* External error handling function */
void interupt(int sig);


int count = 0;
int main(int argc, char *argv[])
{
	int sockTCP                        /* Socket descriptor */
	int sockUDP;
	signal(SIGINT, interupt);          /*Ctrl-c Signal*/
	struct sockaddr_in receptionAddr; /* server address */
	struct sockaddr_in robotAddr;     /* Source address */
	
	
	
	unsigned short serverPort = 5022;     /* server port */
	//unsigned int fromSize;           /* In-out of address size for recvfrom() */
	char *servIP = (char *)malloc(400);                    /* IP address of server */
	char *page = (char *)malloc(SENDMAX);                     /*File location on the host*/
	char *query;                /* query */
	//int messageLen;               /* Length of query */
	int respStringLen;               /* Length of received response */
	struct hostent *thehost;         /* Hostent from gethostbyname() */
	
	int type = 1;
	
	if(strcmp(argv[1], "0") == 0){
	    type = 0;
	}
	
	char *id, *imageID;
	char *tok, *host = (char *)malloc(SENDMAX);
    
    id = argv[3];
    imageID = argv[4];
    
    
    servIP[0] = 0;
    servIP = argv[2];
    serverPort = atoi(argv[1]);
    directory = argv[4];
    char *del = ".edu";
    
    tok = (char *)malloc(200);
    tok = strstr(servIP, del);
    page[0] = '/'; page[1] = 0;
    if(tok == NULL){
        del = ".com";
        tok = strstr(servIP, del);
    }
    if(tok == NULL){
        del = ".org";
        tok = strstr(servIP, del);
    }
    if(tok == NULL){
	    if(strstr(servIP, "http://") != NULL){
		    int i;
		    for(i=7;servIP[i] != '/' && i < 100;i++){
			
		    }
		    memcpy(page, servIP, strlen(servIP));
		    host = servIP;
		
		    page = page + i;
		    host[i] = 0;
	    }
    } else{
        tok = strstr(tok, "/");
        strncpy(host, servIP, strlen(servIP) - strlen(tok));
        host[strlen(servIP) - strlen(tok)] = '\0';


        strcpy(page, tok);
        memcpy(servIP, host, strlen(host) + 1);
    }
	
	
	/*fprintf(stderr, "Page: %s servIP: %s port: 
		%d directory: %s\n", page, servIP, serverPort, directory);*/
	
	
    //fprintf(stderr, "Creating socket\n");
	/* Create a TCP socket */
	if ((sockTCP = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	    DieWithError("ERROR\tSocket Error");
	    
	/* Construct the server address structure */
	memset(&serverAddr, 0, sizeof(serverAddr));/* Zero out structure */
	serverAddr.sin_family = AF_INET;         /* Internet addr family */
	serverAddr.sin_port   = htons(serverPort);     /* Server port */
	
	memset(&robotAddr, 0, sizeof(robotAddr));
	robotAddr.sin_family = AF_INET;
	robotAddr.sin_port = htons(serverPort);
	
	/*Server IP address*/
    robotAddr.sin_addr.s_addr = inet_addr(servIP);

    /*Resolving address if need be*/
    if (robotAddr.sin_addr.s_addr == -1) {
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
        }//end host resolve


    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);













	return 0;
}//end main




void sendUDP(int sock, unsigned char *message, int size, struct sockaddr_in serverAddr){
    
}
char *recvUDP(int sock, struct sockaddr_in allAddress){

}

void sendTCP(int sock, unsigned char *message, int size, struct sockaddr_in serverAddr){
    fprintf(stderr, "Connecting\n");
    if(connect(sock, (struct sockaddr *)
    	&serverAddr, sizeof(serverAddr)) < 0){
        DieWithError("ERROR\tUnable to connect");
    }
    
    
    fprintf(stderr, "Sending\n");
    if ((send(sock, (char *)message, size, 0)) != size){
        DieWithError("ERROR\tSent wrong # of bytes");
    }
    
	fprintf(stderr, "Sent successfully!\n");
}//end sendTCP


char *recvTCP(int sock, struct sockaddr_in serverAddr){
    if (bind(sock, (struct sockaddr *) 
    	&serverAddr, sizeof(serverAddr)) < 0)
        DieWithError("bind() failed");

    /* Mark the socket so it will listen for incoming connections*/
    if (listen(sock, 3) < 0)
        DieWithError("listen() failed");


    
    int csock;                       //Client socket
    unsigned int clntLen;/*Length of client*/
    struct sockaddr_in fromAddr;
    
    clntLen = sizeof(fromAddr);
    
        /* Wait for a client to connect */
    if ((csock = accept(sock, (struct sockaddr *) &fromAddr, 
                       &clntLen)) < 0)
    DieWithError("accept() failed");
    /* Recv a response */ 
    //fromSize = sizeof(fromAddr);
    unsigned char buffer[SENDMAX];
    bzero(buffer, SENDMAX);
    respStringLen = SENDMAX;
    int filesize = SENDMAX;
    int totalrecieved=0;
    unsigned char *content = (unsigned char *) malloc(SENDMAX);
    int contentHead = 0;
    int contentSize = SENDMAX;
    char *modified = (char *) malloc(200);
    
   	
    while(respStringLen > 0){
        if (((respStringLen = 
        	recv(csock,(char *) buffer, SENDMAX, 0)) < 0)){
             if(errno != 11){
                DieWithError("ERROR\tRecieve Error");
                }
        }
        totalrecieved += respStringLen;
        if(totalrecieved > contentSize){
            content = realloc(content, contentSize * 2);
            contentSize = contentSize * 2;
        }
        
        memcpy(content + contentHead, buffer, respStringLen * sizeof(unsigned char));
        contentHead += respStringLen;
    }//loop for all data
    
    return content;            	
}//end recvTCP









void DieWithError(char *errorMessage)
{
    //perror(errorMessage);
    printf("%s\n", errorMessage);
    exit(1);
}//end error die

//Exits on ctrl-c
void interupt(int sig){
    
    
    printf("Recieved %d requests\n", count);
    
    
    
    exit(0);
}//END interupt
