CC=clang++
CFLAGS=-Wall -Werror -std=c++17 -pthread

strace-analyser: strace-analyser.cpp
	$(CC) $(CFLAGS) -o strace-analyser strace-analyser.cpp

clean:
	rm -f *.o strace-analyser *~
