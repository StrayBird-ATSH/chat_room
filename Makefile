OBJS = server.o client.o wrapper.o
CC = gcc
CFLAGS = -Wall -pthread

all: $(OBJS)
	$(CC) $(CFLAGS) server.o wrapper.o -o server
	$(CC) $(CFLAGS) client.o wrapper.o -o client

server:server.o wrapper.o
	$(CC) $(CFLAGS) server.o wrapper.o -o $@
client: client.o wrapper.o
	$(CC) $(CFLAGS) client.o wrapper.o -o $@

$(OBJS): %.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@

server.o: server.c server.h wrapper.c wrapper.h
client.o: client.c client.h wrapper.c wrapper.h
wrapper.o: wrapper.c wrapper.h

clean:
	rm -f $(OBJS) server client
