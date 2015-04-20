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

