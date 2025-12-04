CC=clang
CFLAGS=-Wall -Werror

all: server-monitor server-monitor-linkedlist

server-monitor: server-monitor.o date.o memberlist.o
	$(CC) $(CFLAGS) -o server-monitor server-monitor.o date.o memberlist.o 
server-monitor-linkedlist: server-monitor.o date.o memberlist-linkedlist.o
	$(CC) $(CFLAGS) -o server-monitor-linkedlist server-monitor.o date.o memberlist-linkedlist.o 

date.o: date.h date.c
	$(CC) $(CFLAGS) -o date.o -c date.c

memberlist.o: memberlist.h memberlist.c
	$(CC) $(CFLAGS) -o memberlist.o -c memberlist.c


server-monitor.o: server-monitor.c date.h memberlist.h
	$(CC) $(CFLAGS) -o server-monitor.o -c server-monitor.c

clean:
	rm -f *.o server-monitor server-monitor-linkedlist

       
