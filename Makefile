CC=g++
CFLAGS=-g -std=c++11

all: test

test: test.o
	$(CC) $(CFLAGS) $^ -o $@
	rm test.o

%.o: %.cpp
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	rm test