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
 * License: MIT
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
 */
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <syslog.h>
#include <libconfig.h>

#include "config.h"
#include "utils.h"

void usage();
void version();

int main(int argc, char *argv[]) {	
	int c;
	char* configfile = NULL;
	extern char *optarg;

	/* parse arguments */
	debug = 0;
	while ((c = getopt(argc, argv, "dvf:")) != -1) {
    		switch(c) {
    			case 'v':
				version();
				break;
    			case 'f':
				configfile = optarg;
				break;
			case 'd':
				debug = 1;
				break;
    			case '?':
				usage();
    		}
	}
	if(configfile == NULL) {
		LOGMSG(LOG_ERR,"You need to specify a configuration file\n");
		usage();
	}
	/* open syslog */
	if(!debug) {
		openlog(argv[0], LOG_PID|LOG_CONS, LOG_DAEMON);
	}
	LOGMSG(LOG_INFO,"----- racker is starting -----");
	/* parse config file */
	config_t *cfg = config_initialize(configfile);
	/* read misc config variables */
	config_other(cfg);
	/* fall to background */
	if(!debug) {
		daemonize();
	}
	/* install signal handler - clean closing on shutdown */
	signal(SIGTERM,sig_handler);
	/* configure network listeners */
	config_listeners(cfg);
	return EXIT_SUCCESS;
}

void version() {
        printf("racker-%.2f, © 2009-2010 Matthias Fassl\n\n",VERSION);
	printf("build machine: %s\nbuild date: %s\n",BUILDMACHINE,BUILDDATE);
	exit(EXIT_SUCCESS);
}

void usage() {
	printf("Usage:\n");
	printf("\t-v for version information\n");
	printf("\t-f \"FILE\" specify which configuration file to use (necessary)\n");
	printf("\t-d debug mode\n");
	exit(EXIT_FAILURE);
}
