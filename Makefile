VERSION=0.2
CC = gcc
CFLAGS = -static -g $(shell pkg-config --cflags libconfig) -I inc -DVERSION=$(VERSION) -DBUILDDATE=$(BUILDDATE) -DBUILDMACHINE=$(BUILDMACHINE)
LDFLAGS = $(shell pkg-config --libs libconfig)
TARGET=/usr
BUILDDATE="\"$(shell date +%Y-%m-%d)\""
BUILDMACHINE="\"$(shell uname -mor)\""
OBJ = racker.o config.o utils.o net.o actions.o 

all: $(OBJ)
	$(CC) $(CFLAGS) -o racker $(OBJ) $(LDFLAGS)
	

%.o: src/%.c
	$(CC) $(CFLAGS) -c $< 

clean:
	@rm -rf racker
	@rm -f *.o
