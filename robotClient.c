#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "robots.h"

#define PI 3.141
char *robotID;
char *address;
int L, N;
unsigned int port;
int requestID = 0; 

void getArguments(char **argv);
void takeSnapshot();
requestMsg *makeRequest(char *command);


void moveRobot();
void turnRobot(double angle);
void sendRequest(requestMsg *request);
char *getGPS();
char *getDGPS();
char *getLasers();
void getImage();


int main(int argc, char *argv[]) 
{
    // If we don't have all of the required command line arguments
    if (argc < 6) {
        printf("Usage: %s IP/Host_name serverPort robotID L N\n", argv[0]);
        exit(1);
    }
    getArguments(argv);
    
    double angle = PI * N * (N-2); 
    
    takeSnapshot();
    int turn = 0;
    while (true) {
        // Move the robot L units
        moveRobot(L);
        // Turn the robot angle radians
        turnRobot(angle);
        // Get all of the data from the robot
        takeSnapshot(turn);
        requestID++;
    }
    return 0;
}

void getArguments(char **argv) {
    address = argv[1];
    port = atoi(argv[2]);
    robotID = argv[3];
    L = atoi(argv[4]);
    N = atoi(argv[5]);

}

void takeSnapshot(int turn) {
    char *imageFilename;
    sprintf(imageFilename, "image-%d.png", turn);
    char *textFilename;
    sprintf(textFilename, "position-%d.txt", turn);

    FILE *textFile = fopen(textFilename, "w");
    char *GPS = getGPS();
    char *DGPS = getDGPS();
    char *lasers = getLasers();
    
    getImage();
}
void moveRobot(int units) {
    char command[15];
    sprintf(command, "MOVE %d", units);
    requestMsg *request = makeRequest(command);
    sendRequest(request);
}

void turnRobot(double angle) {
    char command[15];
    sprintf(command, "TURN %f", angle);
    requestMsg *request = makeRequest(command);
    sendRequest(request);
}

void stopRobot() {
    char command[15];
    sprintf(command, "STOP");
    requestMsg *request = makeRequest(command);
    sendRequest(request);
}

char *getGPS() {
    char command[15];
    sprintf(command, "GET GPS");
    requestMsg *request = makeRequest(command);
    sendRequest(request);
}

char *getDGPS() {
    char command[15];
    sprintf(command, "GET DGPS");
    requestMsg *request = makeRequest(command);
    sendRequest(request);
}

char *getLasers() {
    char command[15];
    sprintf(command, "GET LASERS");
    requestMsg *request = makeRequest(command);
    sendRequest(request);
}

// Gets an image from the robot
void getImage() {
    char command[15];
    sprintf(command, "GET IMAGE");
    requestMsg *request = makeRequest(command);
    sendRequest(request);
}

void sendRequest(requestMsg *request) {
    // Send a UDP message to the middleware

}

// Creates a new request.
requestMsg *makeRequest(char *command) {
    requestMsg *newRequest = malloc(sizeof(requestMsg)); 
    newRequest->commID = requestID;
    strcpy(newRequest->robotID, robotID);  
    strcpy(newRequest->command, command);
    return newRequest;
}
