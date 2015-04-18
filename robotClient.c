#include <stdio.h>
#include <stdlib.h>

typedef struct command_message {
    unsigned int commID;
    char robotId[10];
    char robotCommand[10];
} comm_msg;

typedef struct response_message {
    unsigned int commID;
    int messagesToRecv;
    int messageIndex;

}
int main(int argc, char *argv[]) 
{
    // If we don't have all of the required command line arguments
    if (argc < 6) {
        printf("Usage: %s MIP/Host_name> <port> <ID> <L> <N>\n", argv[0]);
        exit(1);
    }
    comm_msg *msg; 
    msg = malloc(sizeof(comm_msg));
    msg->commId = 3
    char *address = argv[1];
    unsigned int port = atoi(argv[2]);
    char *ID = argv[3];
    int L = atoi(argv[4]);
    int N = atoi(argv[5]);

}
