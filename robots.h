typedef struct request_message {
    unsigned int commID;
    char *robotID;
    char *command;
} requestMsg;

#pragma pack(push, 1)
typedef struct response_message {
    unsigned int requestID;
    unsigned int nMessages;
    unsigned int sequenceN;
    void *data;
} responseMsg;
#pragma pack(pop)

enum commands {MOVE, TURN, STOP, IMAGE, GPS, DGPS, LASERS};

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




char *imageAddr = "/snapshot?topic=/robot_5/image?width=600?height=500";//8081
char *action = "/twist?id=2agreeable";//8082 move: &lx=, turn: &az=, stop: &lx=0
char *dGPS = "/state?id=2agreeable";//8084
char *lasers = "/state?id=2agreeable";//8083



