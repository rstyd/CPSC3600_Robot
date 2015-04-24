CC=clang
CFLAGS=-Wall -g -std=c99

BINS=robotClient robotServer

all: $(BINS)

robotClient: robotClient.c
	$(CC) $(CFLAGS) -o robotClient robotClient.c -lm
	
robotServer: robotServer.c
	$(CC) $(CFLAGS) -o robotServer robotServer.c -lm

clean:
	rm robotClient robotServer
	clear

client:
	./robotClient localhost 5022 2agreeable 1 4 

server:
	./robotServer 5022 130.127.192.62 2agreeable 5

package:
	tar cvzf smsouth_daj2_clauded_bobrigg.tar.gz README robots.h robotClient.c robotServer.c Makefile
