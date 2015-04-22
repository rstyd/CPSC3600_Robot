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


struct sockaddr_in *clientAddr;
int sockTCP;                        /* Socket descriptor */
int responseSize;	
int main(int argc, char *argv[])
{
	
	int sockUDP;
	signal(SIGINT, interupt);          /*Ctrl-c Signal*/
	struct sockaddr_in receptionAddr; /* server address */
	struct sockaddr_in robotAddr;     /* Source address */
	
	
	
	unsigned short serverPort = 5022;     /* server port */
	char *servIP = (char *)malloc(400);                    /* IP address of server */
	char *page = (char *)malloc(SENDMAX);                     /*File location on the host*/
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


    receptionAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    while(1){
        requestMsg *newRequest; 
        unsigned char *resp = recvUDP(sockUDP, receptionAddr);
        newRequest = (requestMsg *)resp;
        //TODO: set page according to resp
        char *query = malloc(sizeof(char) * 500);
        query[0] = 0;
        char *page = dGPS;
       	robotAddr.sin_port = htons(8084);

        char *cmd = newRequest->command;

        if(strstr(cmd, "IMAGE") != NULL){
       		page = imageAddr;
       		robotAddr.sin_port = htons(8081);
        }
        if(strstr(cmd, "DGPS") != NULL){
       		page = dGPS;
        }else if(strstr(cmd, "GPS") != NULL){
       		page = action;
       		robotAddr.sin_port = htons(8082);
        }
        if(strstr(cmd, "LASERS") != NULL){
       		page = lasers;
       		robotAddr.sin_port = htons(8083);
        }
        if(strstr(cmd, "MOVE") != NULL){
       		page = action;
       		page = strcat(page, "&lx=");
       		strtok(cmd, " ");
       		char *vel = strtok(NULL, " ");
       		page = strcat(page, vel);
       		robotAddr.sin_port = htons(8082);
        }
        if(strstr(cmd, "TURN") != NULL){
       		page = action;
       		page = strcat(page, "&az=");
       		strtok(cmd, " ");
       		char *vel = strtok(NULL, " ");
       		page = strcat(page, vel);
       		robotAddr.sin_port = htons(8082);
        }
        if(strstr(cmd, "STOP") != NULL){
       		page = action;
       		page = strcat(page, "&lx=0");
       		robotAddr.sin_port = htons(8082);
        }

        sprintf(query, 
        "GET %s HTTP/1.1\r\n"
        "User-Agent: wget/1.14 (linux-gnu)\r\n"
        "Host: %s\r\n"
        "Connection: Keep-Alive\r\n"
        "\r\n", page, host);

       	sendTCP(sockTCP, (unsigned char *)query, strlen(query), robotAddr);
        
        unsigned char *content = recvTCP(sockTCP, robotAddr);

        char *header = (char *)malloc(500);
        header[0] = 0;
        int act = 1, i = 0;
        for(;i<strlen((char *)content);i++){
            header[i] = content[i];
            if(content[i] == '\n' || content[i] == '\r'){
                if(act == 2){
                    header[i+1] = 0;
                    break;
                } else if(act == 0){
                    act = 2;
                } else
                    act = 0;
            } else
                act = 1;
        }//end for
        fprintf(stderr, "%s\n", header);
        
        content = (unsigned char *)strstr((char *)content, "\r\n\r\n");
        if(content == NULL){
            //fprintf(stderr, "Issue with content request\n%s", content);
        }
        content = content + 4;

        struct response_message *rm = (struct response_message *) malloc(sizeof( struct response_message));

        int number = (1000 - sizeof(struct response_message) + sizeof(void *))/responseSize;
        if(number == 0)
       		number = 1;
        rm->nMessages = number; 
        int sequence = 0;
        int keeper = 0;
        while(sequence < number){
            rm->sequenceN = sequence;
            if(sequence == number - 1){
                rm->data = content + (responseSize % 1000 - 
                    sizeof(struct response_message) + sizeof(void *));
                sendUDP(sockUDP, (unsigned char *)rm, (responseSize % 1000 - 
                    sizeof(struct response_message) + sizeof(void *)));
                break;
                
            }
       		
            rm->data = content + keeper;
            keeper += 1000 - sizeof(struct response_message) + sizeof(void *);
            sendUDP(sockUDP, (unsigned char *) rm, 1000);
        }//end sequence loop
    }//end ETERNAL LOOP
	return 0;
}//end main


void sendUDP(int sock, unsigned char *message, int size){
    fprintf(stderr, "Sending response to client\n");
    if (sendto(sock, (char *)message, size, 0, (struct sockaddr *)
        &clientAddr, sizeof(clientAddr)) != size)
            DieWithError("ERROR\tSent wrong # of bytes");
}//end sendUDP==================================================================


unsigned char *recvUDP(int sock, struct sockaddr_in allAddress){
   
    fprintf(stderr,"waiting for command\n");
    /* Recv a response */ 
    unsigned char buffer[SENDMAX];
    bzero(buffer, SENDMAX);
    int respStringLen = SENDMAX;
    int totalrecieved=0;
    unsigned char *content = (unsigned char *) malloc(SENDMAX);
    int contentHead = 0;
    int contentSize = SENDMAX;
    unsigned int clntLen = sizeof(clientAddr);
   	
    while(respStringLen > 0){
        if ((respStringLen = recvfrom(sock, (char *)buffer, SENDMAX, 0, 
             (struct sockaddr *) &clientAddr, &clntLen)) < 0){
            DieWithError("ERROR\tRecieve Error");
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
}//end recvUDP


void sendTCP(int sock, unsigned char *message, int size, struct sockaddr_in serverAddr){
    fprintf(stderr, "Connecting to robot\n");
    if(connect(sock, (struct sockaddr *)
    	&serverAddr, sizeof(serverAddr)) < 0){
        DieWithError("ERROR\tUnable to connect");
    }
    
    
    fprintf(stderr, "Sending\n");
    if ((send(sock, (char *)message, size, 0)) != size){
        DieWithError("ERROR\tSent wrong # of bytes");
    }
    
	fprintf(stderr, "Sent successfully!\n");
}//end sendTCP==================================================================


unsigned char *recvTCP(int sock, struct sockaddr_in serverAddr){
    fprintf(stderr, "getting response from robot\n");
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
    int respStringLen = SENDMAX;
    int totalrecieved=0;
    unsigned char *content = (unsigned char *) malloc(SENDMAX);
    int contentHead = 0;
    int contentSize = SENDMAX;
   	
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
    responseSize = totalrecieved;
    close(sockTCP);
    return content;            	
}//end recvTCP==================================================================

void DieWithError(char *errorMessage)
{
    //perror(errorMessage);
    printf("%s\n", errorMessage);
    exit(1);
}//end error die

//Exits on ctrl-c
void interupt(int sig){
    fprintf(stderr, "\nEnding it all!\n");
    close(sockTCP);
    exit(0);
}//END interupt
