VERSION=0.2
CC = gcc
CFLAGS =  -static -Wall -ansi -pedantic -g $(shell pkg-config --cflags libconfig) -I inc -DVERSION=$(VERSION) -DBUILDDATE=$(BUILDDATE) -DBUILDMACHINE=$(BUILDMACHINE)
LDFLAGS = $(shell pkg-config --libs libconfig)
TARGET=/usr
BUILDDATE="\"$(shell date +%Y-%m-%d)\""
BUILDMACHINE="\"$(shell uname -mor)\""
OBJ = net.o utils.o actions.o config.o racker.o

all: $(OBJ)
	$(CC) $(CFLAGS) -o racker $(OBJ) $(LDFLAGS)
	

%.o: src/%.c
	$(CC) $(CFLAGS) -c $< 

clean:
	@rm -rf racker
	@rm -f *.o
