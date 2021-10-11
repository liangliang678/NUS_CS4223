CC = gcc
CFLAGS = -Wall -g -O2 -lpthread

SRC = main.c MESI.c Dragon.c

all: 
	$(CC) -o coherence $(SRC) $(CFLAGS)

clean:
	rm coherence
