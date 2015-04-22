typedef struct request_message {
    unsigned int commID;
    char *robotID;
    char *command;
} requestMsg;

typedef struct response_message {
    unsigned int nMessages;
    unsigned int sequenceN;
    void *data;
} responseMsg;

enum commands {MOVE, TURN, STOP, IMAGE, GPS, DGPS, LASERS};

<<<<<<< HEAD
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>     /* for strlen() and memcpy*/
#include <netinet/in.h> /* for in_addr */
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <netdb.h>      /* for getHostByName() */
#include <stdlib.h>     /* for atol() and exit() */
#include <unistd.h>     /* for close() */
#include <stdbool.h>
#include <time.h>
=======


void sendUDP(int sock, unsigned char *message, int size);
unsigned char *recvUDP(int sock, struct sockaddr_in serverAddr);

void sendTCP(int sock, unsigned char *message, int size, struct sockaddr_in robotAddr);
unsigned char *recvTCP(int sock, struct sockaddr_in serverAddr);


char *imageAddr = "/snapshot?topic=/robot_5/image?width=600?height=500";//8081
char *action = "/state?id=2agreeable";//8082 move: &lx=, turn: &az=, stop: &lx=0
char *dGPS = "/state?id=2agreeable";//8084
char *lasers = "/state?id=2agreeable";//8083

>>>>>>> 529556945dac72f6cd2fd82981e8362369716018


