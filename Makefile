CC = gcc
CFLAGS =  $(shell mysql_config --cflags) $(shell pkg-config --cflags lua) -I inc
LIBS = $(shell mysql_config --libs) $(shell pkg-config --libs lua)  -pthread
TARGET=/usr
VERSION=0.2
BUILDDATE="\"$(shell date +%Y-%m-%d)\""
BUILDMACHINE="\"$(shell uname -mor)\""

mysql:
	$(CC) -c src/mysql.c $(CFLAGS)
net:
	$(CC) -c src/net.c

utils:
	$(CC) -c src/utils.c

actions:
	$(CC) -c src/actions.c

config:
	$(CC) -c src/config.c $(CFLAGS)

main:
	$(CC) -c src/racker.c -DVERSION=$(VERSION) -DBUILDDATE=$(BUILDDATE) -DBUILDMACHINE=$(BUILDMACHINE)

all: mysql net utils actions config main
	$(CC) -o racker *.o $(CFLAGS) $(LIBS)
	
install : all
	@echo creating necessary user for tracker
	@if [ -n "$(shell id racker)" ]; then userdel racker;fi
	@useradd racker
	@echo installing executable file to $(TARGET)/bin
	@mkdir -p $(TARGET)/bin
	@cp -f racker $(TARGET)/bin
	@chmod 755 $(TARGET)/bin/racker
	@echo installing init script to /etc/init.d
	@cp -f racker.init /etc/init.d/racker
	@echo installing default file to /etc/default
	@cp -f racker.default /etc/default/racker
	@echo installing config file to /etc/racker.conf
	@cp -f racker.conf /etc/racker.conf

uninstall: clean
	@rm -rf /etc/racker.conf
	@rm -rf /etc/default/racker
	@rm -rf /etc/init.d/racker
	@rm -rf /var/run/racker.pid
	@rm -rf $(TARGET)/bin/racker

clean:
	@rm -rf racker
	@rm -f *.o

release: clean
	@mkdir racker
	@cp *.c racker/
	@cp *.h racker/
	@cp racker.* racker/
	@cp Makefile racker/
	@cp INSTALL racker/
	@tar cvzf racker-$(shell date +%Y%m%d).tar.gz racker	
	@rm -rf racker
	@mkdir -p releases
	@mv racker-$(shell date +%Y%m%d).tar.gz releases/

pubrelease: release
	@scp releases/racker-$(shell date +%Y%m%d).tar.gz boltzmann.x1598.at:/home/matthias/public_html/

data: all
	@mkdir -p debian/usr/bin
	@cp -f racker debian/usr/bin
	@mkdir -p debian/etc/default
	@mkdir -p debian/etc/init.d
	@cp -f racker.default debian/etc/default/racker
	@cp -f racker.init debian/etc/init.d/racker
	@cp -f racker.conf debian/etc
	@mkdir -p debian/usr/share/man/man1
	@cp -f racker.1.gz debian/usr/share/man/man1
	@mkdir -p debian/usr/share/doc/racker
	@cp -f racker.sql debian/usr/share/doc/racker/

debian: data
	@mkdir -p debian/DEBIAN
	@cp -f racker.control debian/DEBIAN/control
	@dpkg-deb --build debian racker_$(VERSION)_x86.deb
	@rm -rf debian
