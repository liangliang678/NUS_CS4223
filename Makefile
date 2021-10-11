CC = gcc
CFLAGS = -Wall -g -O2

SRC = main.c

all: 
	$(CC) -o coherence $(SRC) $(CFLAGS)

clean:
	rm coherence
