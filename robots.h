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


