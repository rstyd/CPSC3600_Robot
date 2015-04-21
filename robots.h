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




void sendUDP(int sock, unsigned char *message, struct sockaddr_in serverAddr);
char *recvUDP(int sock, struct sockaddr_in allAddress);

void sendTCP(int sock, char *message, struct sockaddr_in serverAddr);
char *recvTCP(int sock, struct sockaddr_in fromAddr);

