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



void sendUDP(int sock, unsigned char *message, int size, struct sockaddr_in serverAddr);
char *recvUDP(int sock, struct sockaddr_in serverAddr);

void sendTCP(int sock, char *message, int size, struct sockaddr_in serverAddr);
char *recvTCP(int sock, struct sockaddr_in serverAddr);


char *imageAddr = "/snapshot?topic=/robot_5/image?width=600?height=500";//8081
char *action = "/state?id=2agreeable";//8082 move: &lx=, turn: &az=, stop: &lx=0
char *dGPS = "/state?id=2agreeable";//8084
char *lasers = "/state?id=2agreeable";//8083



