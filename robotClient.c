#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "robots.h"
#include <sys/time.h>
#include <signal.h>

#define PI 3.141
char *robotID;
char *serverIP;
int L, N;
unsigned short port;
int requestID = 0; 

volatile sig_atomic_t waiting = 1;

void getArguments(char **argv);
void takeSnapshot();
requestMsg *makeRequest(char *command);

void AlarmHandler(int sig) {
    puts("ALARMED BITCHES");
    waiting = 0;
}

void moveRobot();
void stopRobot();
void startTimer(int seconds);
void turnRobot(double angle);
void sendRequest(requestMsg *request);
int getGPS(unsigned char **data);
int getDGPS(unsigned char **data);
int getLasers(unsigned char **data);

int getImage(unsigned char **data);
int recvRequest(unsigned char **data);
int sock;

struct sockaddr_in middlewareAddr;  // Local Address
struct sockaddr_in fromAddr;       // Client Address
void DieWithError(char *errMsg) {
    fprintf(stderr, "%s\n", errMsg);
    exit(1);
}

int main(int argc, char *argv[]) 
{
    signal(SIGALRM, AlarmHandler);
    struct hostent *thehost;         // Hostent from gethostbyname()
    serverIP = argv[1];
    port = atoi(argv[2]);
    robotID = argv[3];
    L = atoi(argv[4]);
    N = atoi(argv[5]);

    printf("Server IP: %s Port %d robotID: %s L %d N %d\n", serverIP, port, robotID, L, N);
    // If we don't have all of the required command line arguments

    if (argc < 6) {
        printf("Usage: %s IP/Host_name serverPort robotID L N\n", argv[0]);
        exit(1);
    }
    //
    // Construct the server address structure
    // zero out address structure
    memset(&middlewareAddr, 0, sizeof(middlewareAddr));
    // Set the internet address family
    middlewareAddr.sin_family = AF_INET;
    // Server IP Address
    middlewareAddr.sin_addr.s_addr = inet_addr(serverIP);
    // Server port
    middlewareAddr.sin_port = htons(port);

    // Get the actual name of the host if given an IP address
    if (middlewareAddr.sin_addr.s_addr == -1){
        thehost = gethostbyname(serverIP);
        middlewareAddr.sin_addr.s_addr = *((unsigned long *) thehost->h_addr_list[0]);
    }


    if((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0){
        perror("socket() failed");
        return -1;
    }

    double angle = (PI * (N-2))/N; 

    int turn = 0;
    //takeSnapshot(turn);
    // Draw the first Polygon
    for (int i = 0; i < N; i++) {
        printf("On turn: %d", i);
        // Move the robot L meters in the current direction then stop
        puts("MOVING ROBOT");
        moveRobot(L);
        // Turn the robot angle radians then stop
        puts("TURNING ROBOT");
        turnRobot(angle);
        // Get all of the data from the robot
        requestID++;
        puts("TAKING SNAPSHOT");
        //takeSnapshot(turn);
        turn++;
    }
    return 0;
}

void startTimer(int seconds) {
    alarm(seconds);
}

void takeSnapshot(int turn) {
    puts("Taking snapshot");
    char imageFilename[15];
    sprintf(imageFilename, "image-%d.png", turn);
    char textFilename[15];
    sprintf(textFilename, "position-%d.txt", turn);

    FILE *textFile = fopen(textFilename, "wb");
    unsigned char  *GPS, *DGPS, *lasers;
    int gpsSize = getGPS(&GPS);
    printf("%d\n", gpsSize);
    int dgpsSize = getDGPS(&DGPS);
    int lasersSize= getLasers(&lasers);
    printf("gps %d  dgps %d lasers %d\n", gpsSize, dgpsSize, lasersSize);
 /*   fprintf(textFile, "GPS "); 
    fwrite(GPS, gpsSize, 1, textFile);
    fprintf(textFile, "\ndGPS "); 
    fwrite(DGPS, dgpsSize, 1, textFile);
    fprintf(textFile, "\nlasers "); 
    fwrite(lasers, lasersSize, 1, textFile);
    fprintf(textFile, "\n"); 
    fclose(textFile);
*/
    unsigned char *data;
    int imageSize = getImage(&data);
    printf("Image Size: %d\n", imageSize);
    FILE *imageFile = fopen(imageFilename, "wb"); 
 //   fwrite(data, imageSize, 1, imageFile);
    fclose(imageFile);
}
void moveRobot(int meters) {
    puts("Moving robot");
    // Compute the speed required for moving L meters in 7 seconds
    int moveTime = 5;
    double speed = (meters * 1.0)/moveTime;
    speed = -1.0;
    char command[15];
    

    sprintf(command, "MOVE %f", speed);    
    requestMsg *request = makeRequest(command);
    puts("Sending request"); 
    sendRequest(request);
    void *data;
    int size = recvRequest(data);
    puts("Reciveing request");
    puts("Starting wait");
    // Waits for the movement time to go off 
    sleep(5); 
    puts("DONE");
    stopRobot(); 
}

void turnRobot(double angle) {
    puts("Turning Robot");
    char command[15];
    sprintf(command, "TURN %f", angle);
    int moveTime = 7;
    double speed = angle/moveTime;
    requestMsg *request = makeRequest(command);
    sendRequest(request);
    stopRobot();
}

void stopRobot() {
    puts("Stopping robot");
    char command[15];
    sprintf(command, "STOP");
    requestMsg *request = makeRequest(command);
    sendRequest(request);
}

int getGPS(unsigned char **data) {
    puts("Getting GPS");
    char command[15];
    sprintf(command, "GET GPS");
    requestMsg *request = makeRequest(command);
    sendRequest(request);
    int dataSize = recvRequest(data);
    return dataSize;
}

int getDGPS(unsigned char **data) {
    puts("Getting DGPS");
    char *dgpsData = malloc(30);
    char command[15];
    sprintf(command, "GET DGPS");
    requestMsg *request = makeRequest(command);
    sendRequest(request);
    int dataSize = recvRequest(data);
    return dataSize;
}

int getLasers(unsigned char **data) {
    puts("Getting lasers");
    char *laserData = malloc(30);
    char command[15];
    sprintf(command, "GET LASERS");
    requestMsg *request = makeRequest(command);
    sendRequest(request);
    int dataSize = recvRequest(data);
    return dataSize;
}

// Gets an image from the robot
int getImage(unsigned char **data) {
    puts("Getting image");
    char command[15];
    sprintf(command, "GET IMAGE");
    requestMsg *request = makeRequest(command);
    sendRequest(request);
    int dataSize = recvRequest(data);
    return dataSize;
}

void resetClock() {

}

void sendRequest(requestMsg *request) {
    // Send a UDP message to the middleware
    int sent;
    size_t bufSize = sizeof(unsigned int) + strlen(request->robotID) + 1 + strlen(request->command + 1);
    char *requestBuffer = malloc(bufSize);
    memcpy(requestBuffer, &request->commID, 4);
    memcpy(requestBuffer + 4, request->robotID, strlen(request->robotID) + 1);
    memcpy(requestBuffer + 4 + strlen(request->robotID) + 1, request->command, strlen(request->command) + 1);

    int requestLen = bufSize; 
    // Send the guess
    if ((sent = sendto(sock, requestBuffer, requestLen + 1, 0, (struct sockaddr *)
                    &middlewareAddr, sizeof(middlewareAddr))) != requestLen + 1)
    {
        printf("SENT: %d %zu\n", sent, bufSize);
        DieWithError("sendto() sent a different number of bytes than expected");
    }
    printf("Bytes: %d sent request\n", sent);

}
// Creates a new request.
requestMsg *makeRequest(char *command) {
    requestMsg *newRequest = malloc(sizeof(requestMsg)); 
    newRequest->commID = requestID++;
    newRequest->robotID = malloc(strlen(robotID) + 1);
    strcpy(newRequest->robotID, robotID);  
    newRequest->command = malloc(strlen(command) + 1);
    strcpy(newRequest->command, command);
    return newRequest;
}




int recvRequest(unsigned char **data){
    //return array of response msg instead???
    puts("Running recvRequest");
    responseMsg *messages[1000];  //array of response messages from server

    for (int i = 0; i < 1000; i++) {
        messages[i] = NULL;
    }

    char returnBuffer[1000];
    int respStringLen = 0;

    unsigned int fromSize = sizeof(fromAddr);
    int nMessages = -1;
    // Set the timeout
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,&timeout,sizeof(timeout)) < 0) {
        DieWithError("Could not set timeout");
    }
    int size = 0;
    int messagesRcvd = 0;
    while (true) {
        if (nMessages == messagesRcvd) {
            puts("GOT a MESSAGE");
            break;
        }
        memset(returnBuffer, 0, 1000);
        int respStringLen = 0;

        if ((respStringLen = recvfrom(sock, returnBuffer, 1000, 0,
                        (struct sockaddr *) &fromAddr, &fromSize)) < 1) 
        {
            // Check to see if the socket timedout
            if (errno == EAGAIN) {
                errno = 0;
                break; 
            }
            else
                DieWithError("recvfrom() failed");
        }

        size += respStringLen - 12;
        responseMsg *msg = malloc(sizeof(responseMsg));     
        memcpy(&msg->requestID, &returnBuffer, 4);
        memcpy(&msg->nMessages, returnBuffer + 4, 4);
        memcpy(&msg->sequenceN, returnBuffer + 8, 4);
        printf("ID: %d SEQ: %d nMessages: %d\n", msg->requestID, msg->sequenceN, msg->nMessages);
        msg->data = malloc(988);
        printf("LENGTH %d\n", respStringLen);

        memcpy(msg->data, returnBuffer + 12, respStringLen - 12);
        int nSequence = msg->sequenceN; 
        if (nMessages == -1)  {
            puts("SETTING nMessages");
            nMessages = msg->nMessages; 
            if (nMessages > 1000) {
                realloc(messages, nMessages * 1000); 
            }
        }
        messages[msg->sequenceN] = msg;
        messagesRcvd++;
    }
     
     
    for (int i = 0; i < nMessages; i++) {
        if (messages[i] == NULL) {
            fprintf(stderr, "Did not get all the messages required");
            exit(1); 
        }
    }
    
    printf("Size: %d\n", size);
    
    *data = malloc(size);
    int originalSize = size; 
     int offset = 0;
     for (int i = 0; i < nMessages; i++) {
        if (size < 988)
          memcpy(*data + offset, messages[i]->data, size);
        else  {
          memcpy(*data + offset, messages[i]->data, 988);
          size -= 988;
          offset += 988;
        }
    }
 
    return originalSize;
}

void recvLarge(){
    //--------variables-----------//
    int order; //message number we should be on or count
    int recvSize; //size of received message
    time_t start_t, end_t;
    double diff_t;
    responseMsg messages[100];  //array of response messages from server
    //----------------------------//
    memset(messages,0,sizeof(responseMsg)*100); //zero out array and make space
    //
    //   memset(buffer, 0, bufferSize);
    //
    //time(&start_t);
    while(1){
        puts("SDSDD");
        while (true) {
            int respStringLen = 0;
            char returnBuffer[100];
            unsigned int fromSize = sizeof(fromAddr);
            if ((respStringLen = recvfrom(sock, returnBuffer, 100, 0,
                            (struct sockaddr *) &fromAddr, &fromSize)) < 1) 
            {
                // Check to see if the socket timedout
                if (errno == EAGAIN) {
                    errno = 0;
                    continue;
                }
                else
                    DieWithError("recvfrom() failed");
            }

        }
    }
}

void recvAckno(int timeout){}

char* recvSmall(){
    int recvSize;
    responseMsg messy;
    char buff[1000];
    time_t start_t, end_t;
    double diff_t; 

    time(&start_t);
    time(&end_t);
    /*
       while((diff_t = difftime(end_t, start_t)) < timeout){
       if (recvSize = recvfrom(sock, buff, 1000, 0, 
       (struct sockaddr *) &fromAddr) < 0){
       fprintf(stderr, "recv() less than 0 bytes error or done");
    //break;
    }
    else {
    //insert buffer into struct and take data into string and return
    break;
    }
    time(&end);
    }
    */
}


