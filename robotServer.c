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
	int sock;                        /* Socket descriptor */
	signal(SIGINT, interupt);          /*Ctrl-c Signal*/
	struct sockaddr_in serverAddr; /* server address */
	struct sockaddr_in fromAddr;     /* Source address */
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
	
	
	char *directory = (char *)malloc(100);
	directory[0] = 0;
	char *tok, *host = (char *)malloc(SENDMAX);

	if(type == 0){//client
	    servIP[0] = 0;
	    servIP = argv[2];
	    serverPort = atoi(argv[3]);
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
	} else{//server

	    serverPort = atoi(argv[2]);
	    
	    
	    directory = argv[3];
	}
	
	/*fprintf(stderr, "Page: %s servIP: %s port: 
		%d directory: %s\n", page, servIP, serverPort, directory);*/
	
	
    //fprintf(stderr, "Creating socket\n");
	/* Create a TCP socket */
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	    DieWithError("ERROR\tSocket Error");
	    
	/* Construct the server address structure */
	memset(&serverAddr, 0, sizeof(serverAddr));/* Zero out structure */
	serverAddr.sin_family = AF_INET;         /* Internet addr family */
	serverAddr.sin_port   = htons(serverPort);     /* Server port */
	
	if(type == 0){
		/*Server IP address*/
	    serverAddr.sin_addr.s_addr = inet_addr(servIP);
	
        /*Resolving address if need be*/
        if (serverAddr.sin_addr.s_addr == -1) {
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
                        serverAddr.sin_port = htons(atoi(p));
                    }
                    thehost = gethostbyname(host);
                    printf("%s\n", host);
                }
            
                if(thehost == NULL){
                	//fprintf(stderr, "\n%s\n", servIP);
                    DieWithError("ERROR\tcan't resolve name");
                }
                
                serverAddr.sin_addr.s_addr = 
                	*((unsigned long *) thehost->h_addr_list[0]);
            }//end host resolve
	
	}else{
	    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	
	
	if(type == 0){
	    fprintf(stderr, "Connecting\n");
	    if(connect(sock, (struct sockaddr *)
	    	&serverAddr, sizeof(serverAddr)) < 0){
	        DieWithError("ERROR\tUnable to connect");
	    }
	} else{
	    if (bind(sock, (struct sockaddr *) 
	    	&serverAddr, sizeof(serverAddr)) < 0)
            DieWithError("bind() failed");

        /* Mark the socket so it will listen for incoming connections*/
        if (listen(sock, 3) < 0)
            DieWithError("listen() failed");
	}
    
    
    if(type == 1){
    ///======================================================RECEPTION
	    
	    int csock;                       //Client socket
	    unsigned int clntLen;/*Length of client*/
	    char *name = strtok(directory, ".");
	    char *ext = strtok(NULL, ".");
	    //struct sockaddr_in fromAddr;     /* Source address */
	    clntLen = sizeof(fromAddr);
	    while(1){
                /* Wait for a client to connect */
            if ((csock = accept(sock, (struct sockaddr *) &fromAddr, 
                               &clntLen)) < 0)
            DieWithError("accept() failed");
	        /* Recv a response */ 
	        //fromSize = sizeof(fromAddr);
	        char buffer[SENDMAX];
	        bzero(buffer, SENDMAX);
	        respStringLen = SENDMAX;
	        int filesize = SENDMAX;
	        int totalrecieved=0;
	        char *content = (char *) malloc(SENDMAX);
	        content[0] = 0;
	        char *modified = (char *) malloc(200);
	        
	        if(strcmp(directory, "stdout") != 0){
	       		sprintf(modified, "%s%d.%s", name, count, ext);
	       		//fprintf(stderr, "Printing to %s\n", modified);
	       	}
	       	FILE *file = fopen(modified, "w+");
	       	if(file == NULL){
		       		DieWithError("Can't open file");
		       	}
	        while(respStringLen > 0){
	            if (((respStringLen = 
	            	recv(csock,buffer, SENDMAX, 0)) < 0)){
	                 if(errno != 11){
	                    DieWithError("ERROR\tRecieve Error");
	                    }
	            }
	            totalrecieved += respStringLen;
	            if(totalrecieved > filesize){
	                filesize = filesize + respStringLen;
	                content = realloc(content, filesize);
	                //fprintf(stderr, "Resizing to %d\n", filesize);
	            }
	            
	            if(strcmp(directory, "stdout") != 0){
	            	fwrite (buffer , sizeof(char), respStringLen, file);
	            }else
	            	printf("%s\n",buffer);
	            //strcat(content, buffer);
	        }
	        
	       	fclose(file);
	       	
            count++;
	    }//success/not success loop
	    close(sock);
	} else{//SENDING===================================================
	    long size;
	    FILE *file = fopen(directory, "rb");
	    fseek (file , 0 , SEEK_END);
        size = ftell (file);
        rewind (file);
        
       	if(file == NULL){
       		DieWithError("Can't open file");
       	}
       	
   	    query = (char *)malloc(size * sizeof(char));
   	    fread (query,1,size,file);
   	    fclose(file);
        
        fprintf(stderr, "Sending\n");
        if ((send(sock, query, size, 0)) != size){
            DieWithError("ERROR\tSent wrong # of bytes");
        }
        //fprintf(stderr, "    Sent\n=====\n%s=====\n", query);
        
		printf("Sent file successfully!\n");
	}//end send/recieve
	return 0;
}//end main












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
