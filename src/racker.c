/* racker.c
 *
 * racker (german: rascal, mischievous little kid)
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
 * UDP Bittorrent Tracker implemented after 
 * Olaf van der Spek Protocol Design
 * http://www.rasterbar.com/products/libtorrent/udp_tracker_protocol.html
 * http://xbtt.sourceforge.net/udp_tracker_protocol.html
 * http://www.bittorrent.org/beps/bep_0015.html
 * IPv6: http://opentracker.blog.h3q.com/2007/12/28/the-ipv6-situation/
 *
 * Author: Matthias Fassl <mf@x1598.at>
 * License: AFLv3
 *
 * Version: 0.01 (2009-Feb-01) shows Connection_id, action and Transaction_id of incoming requests
 * Version: 0.02 (2009-Mar-23) answers connect requests successfully
 * Version: 0.03 (2009-Mar-24) checks correct connection_id on connect and sends error messages
 * Version: 0.04 (2009-Mar-25) announce function can write new info into the database
 * Version: 0.10 (2009-Mar-26) announce works! tracker is functional
 *
 * Version: 0.11 (2009-Mar-27) storing ip in the systems byte order, fixed the amount of peers that are returned
 * Version: 0.12 (2009-Mar-30) database entries older than 2*INTERVAL get єrased automatically
 * Version: 0.13 (2009-Mar-30) the first іnfo_hash from scrape gets answered
 * Version: 0.14 (2009-Apr-03) scrape works for all info_hashes that are asked for - can't return num of completed yet
 * Version: 0.15 (2009-Apr-04) support for the ipv6 announce - no support for running the server on ipv6 YET
 * Version: 0.16 (2009-Apr-04) Everything gets logged to syslog, created init script, racker gets daemonized + drops root rights
 * Version: 0.20 (2009-Apr-05) IPv6 support (not yet fully tested) - code clean up, new structure, easy exchange of database backend possible
 *
 * TODO:	
 *
 * Nice-to-Haves
 * 		Authentication
 * 		Check if connection_id is known
 *
 * 		Database support for at least postgres (sqlite?)
 * 		using dynamic link loader (dlopen) to load the right shared object
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <syslog.h>

#include "config.h"
#include "actions.h"
#include "utils.h"
#include "net.h"
#include "database.h"

char* get_config_file(int argc, char* argv[]);
void usage();

int main(int argc, char *argv[]) {	

	char* config;
	
	//open syslog
	openlog(argv[0], LOG_PID|LOG_CONS, LOG_DAEMON);
	syslog(LOG_INFO,"----- racker is starting -----");

	//read config file location from option
	config = get_config_file(argc,argv);	
	//read database config and connect to database
	config_database(config);
	//read misc config variables 
	config_other(config);
	//fall to background
	daemonize();

	//install signal handler - clean closing on shutdown
	signal( SIGTERM, sig_handler );

	config_listeners(config);

	return 0;
}

char* get_config_file(int argc, char* argv[]) {

	int c, file=-1;
	char *filename;
	extern char *optarg;
	extern int optind, optopt, opterr;

	while ((c = getopt(argc, argv, ":vf:")) != -1) {
    		switch(c) {
    			case 'v':
        			printf("racker-%.2f, © 2009 Matthias Fassl\n\n",VERSION);
				printf("build machine: %s\nbuild date: %s\n",BUILDMACHINE,BUILDDATE);
				exit(0);
    			case 'f':
        			filename = optarg;
				return filename;
   			 case ':':
				usage();
    			case '?':
				usage();
    		}
	}
	usage();
}

void usage() {

	printf("Usage:\n\n");
	printf("'racker -v' for Version\n");
	printf("'rakcer -f /path/to/racker.conf' to start daemon\n");
	exit(1);
}
