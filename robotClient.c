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

void moveRobot();
void stopRobot();
void startTimer(int seconds);
void turnRobot(double angle);
void sendRequest(requestMsg *request);

unsigned char *getGPS(int *size);
unsigned char *getDGPS(int *size);
unsigned char *getLasers(int *size);
unsigned char *getImage(int *size);

unsigned char *recvRequest(int *size);
int sock;

struct sockaddr_in middlewareAddr;  // Local Address
struct sockaddr_in fromAddr;       // Client Address
void DieWithError(char *errMsg) {
    fprintf(stderr, "%s\n", errMsg);
    exit(1);
}

int main(int argc, char *argv[]) 
{
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
   // takeSnapshot(turn);
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
    moveRobot(0); 
    
/*
    for (int i = 0; i < N - 1; i++) {
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
*/
    return 0;
}


void takeSnapshot(int turn) {
    puts("Taking snapshot");
    char imageFilename[15];
    sprintf(imageFilename, "image-%d.png", turn);
    char textFilename[15];
    sprintf(textFilename, "position-%d.txt", turn);

    FILE *textFile = fopen(textFilename, "wb");
    char  *GPS, *DGPS, *lasers;
    int gpsSize; 
    GPS = getGPS(&gpsSize);
    printf("%d\n", gpsSize);
    int dgpsSize; 
    DGPS = getDGPS(&dgpsSize);
    int lasersSize;
    lasers = getLasers(&lasersSize);
    printf("%d gpsSize %d dgpsSize %d lasersSize\n", gpsSize, dgpsSize, lasersSize);
    printf("gps %d  dgps %d lasers %d\n", gpsSize, dgpsSize, lasersSize);
    puts("GPS DATA");
    for (int i = 0; i < gpsSize; i++) {
        printf("%c", GPS[i]);
    }
    char *textData = malloc(gpsSize + dgpsSize + lasersSize); 
    char *gpsPreface = "GPS ";
    char *dgpsPreface = "dGPS ";
    char *lasersPreface = "Lasers ";
    char *newLine = "\n";

    fprintf(textFile, "GPS %s\n DGPS %s\n Lasers %s\n", GPS, DGPS, lasers);
    fclose(textFile);

    unsigned char *data;
    int imageSize; 
    data = getImage(&imageSize);
    printf("Image Size: %d\n", imageSize);
    FILE *imageFile = fopen(imageFilename, "wb"); 
    fwrite(data, 1, imageSize, imageFile);
    fclose(imageFile);

}
void moveRobot(int meters) {
    puts("Moving robot");
    // Compute the speed required for moving L meters in 7 seconds
    int moveTime = 5;
    double speed = (meters * 1.0)/moveTime;
    char command[15];
    

    sprintf(command, "MOVE %f", speed);    
    requestMsg *request = makeRequest(command);
    puts("Sending request"); 
    sendRequest(request);
    unsigned char *data;
    int size;
    data = recvRequest(&size);
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
    int moveTime = 5;
    angle = PI  - angle;
    double speed = angle/moveTime;
    sprintf(command, "TURN %f", speed);
    requestMsg *request = makeRequest(command);

    sendRequest(request);
    unsigned char *data;
    int size;
    data = recvRequest(&size);
    sleep(moveTime);
    stopRobot();
}

void stopRobot() {
    puts("Stopping robot");
    char command[15];
    sprintf(command, "STOP");
    requestMsg *request = makeRequest(command);
    sendRequest(request);
    unsigned char *data;
    int size;
    data = recvRequest(&size);

}

unsigned char *getGPS(int *size) {
    puts("Getting GPS");
    char command[15];
    sprintf(command, "GET GPS");
    requestMsg *request = makeRequest(command);
    sendRequest(request);
    unsigned char *data  = recvRequest(size);
    return data;
}

unsigned char *getDGPS(int *size) {
    puts("Getting DGPS");
    char command[15];
    sprintf(command, "GET DGPS");
    requestMsg *request = makeRequest(command);
    sendRequest(request);
    unsigned char *data = recvRequest(size);
    return data;
}

unsigned char *getLasers(int *size) {
    puts("Getting lasers");
    char command[15];
    sprintf(command, "GET LASERS");
    requestMsg *request = makeRequest(command);
    sendRequest(request);
    unsigned char *data = recvRequest(size);
    return data;
}

// Gets an image from the robot
unsigned char *getImage(int *size) {
    puts("Getting image");
    char command[15];
    sprintf(command, "GET IMAGE");
    requestMsg *request = makeRequest(command);
    sendRequest(request);
    unsigned char *data = recvRequest(size);
    return data;
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
    free(requestBuffer);
    printf("Bytes: %d sent request\n", sent);

}
// Creates a new request.
requestMsg *makeRequest(char *command) {
    requestMsg *newRequest = malloc(sizeof(requestMsg)); 
    newRequest->commID = requestID++;
    printf("ID: %d", newRequest->commID);
    newRequest->robotID = malloc(strlen(robotID) + 1);
    strcpy(newRequest->robotID, robotID);  
    newRequest->command = malloc(strlen(command) + 1);
    strcpy(newRequest->command, command);
    return newRequest;
}




unsigned char *recvRequest(int *size){
    //return array of response msg instead???
    puts("Running recvRequest");
    responseMsg *messages[1000];  //array of response messages from server

    for (int i = 0; i < 1000; i++) {
        messages[i] = NULL;
    }
    unsigned char *data;
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
    int fileSize = 0;
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

        fileSize += respStringLen - 12;
        responseMsg *msg = malloc(sizeof(responseMsg));     
        msg->data = malloc(988);
        memcpy(&msg->requestID, returnBuffer, 4); 
        memcpy(&msg->nMessages, returnBuffer + 4, 4);
        memcpy(&msg->sequenceN,returnBuffer + 8,  4);
        memcpy(msg->data, returnBuffer + 12, respStringLen - 12);
        
        printf("ID: %d SEQ: %d nMessages: %d\n", msg->requestID, msg->sequenceN, msg->nMessages);
        printf("LENGTH %d\n", respStringLen);
        printf("STRING LANGTH%d\n", respStringLen - 12);
        printf("%s\n", (char *) returnBuffer + 12);
        if (nMessages == -1)  {
            puts("SETTING nMessages");
            nMessages = msg->nMessages; 
            if (nMessages > 1000) {
                puts("that's a lot of messages"); 
               // messages = realloc(messages, nMessages * 1000); 
            }
        }
        messages[msg->sequenceN] = msg;
        messagesRcvd++;
    }
     
     
    for (int i = 0; i < nMessages; i++) {
        if (messages[i] == NULL) {
            fprintf(stderr, "Did not get all the messages required\n");
            exit(1); 
        }
    }
    
    printf("Recieved Filesize: %d\n", fileSize);
    
    data = malloc(fileSize);
    *size = fileSize; 
     int offset = 0;
     for (int i = 0; i < nMessages; i++) {
        if (fileSize < 988)
          memcpy(data + offset, messages[i]->data, fileSize);
        else  {
          memcpy(data + offset, messages[i]->data, 988);
          fileSize -= 988;
          offset += 988;
        }
    }
 
    return data;
}

