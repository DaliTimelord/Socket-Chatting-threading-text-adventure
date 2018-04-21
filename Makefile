CC = gcc
CFLAGS = -w
LDFLAGS = -pthread -lnsl -lrt 

all: xchatserver xchatclient 

xchatserver: chatserver.c nethelp.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ chatserver.c nethelp.c

xchatclient: chatclient.c nethelp.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ chatclient.c nethelp.c

clean:
	rm -f xchatserver xchatclient *.o *~ core