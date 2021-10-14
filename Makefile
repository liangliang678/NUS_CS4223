CC = gcc
CFLAGS = -Wall -g -O2 -lpthread

SRC = main.c MESI.c Dragon.c bus.c

all: 
	$(CC) -o coherence $(SRC) $(CFLAGS)

clean:
	rm coherence output/*
