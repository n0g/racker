#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include <libconfig.h>

#include "config.h"
#include "net.h"
#include "utils.h"

config_t* config_initialize(char* filename) {
	config_t *config = malloc(sizeof(config_t));
	if(config != NULL) {
		config_init(config);
		if(config_read_file(config,filename) == CONFIG_FALSE) {
			fprintf(stderr,"%s on line %d\n",config_error_text(config),config_error_line(config));
			config_destroy(config);
			exit(EXIT_FAILURE);	
		}
	}
	return config;
}
	
void config_listeners(config_t *config) {
	config_setting_t *interfaces,*interface;
	const char *type,*hostname;
	int portnr, interface_num, sock;

	/* read interface settings */
	if((interfaces = config_lookup(config,"interfaces")) == NULL) {
		fprintf(stderr,"Couldn't read Interfaces\n");
	}
	
	interface_num = config_setting_length(interfaces);
	while(interface_num--) {
		interface = config_setting_get_elem(interfaces,interface_num);

		if(!(config_setting_lookup_string(interface,"type",&type) &&
		config_setting_lookup_string(interface,"hostname",&hostname) &&
		config_setting_lookup_int(interface,"port",(long*) &portnr))) {
			continue;
		}
		/* bind to interface */
		if(strcmp(type,"IPv4") == 0) {
			sock = bind4(hostname,portnr);
		} else if(strcmp(type,"IPv6") == 0) {
			sock = bind6(hostname,portnr);
		}
		/* add interface to some kind of global list */
	}
	/* call aproppriate send/receive loop */
}

void config_other(config_t *config) {
	if(!(config_lookup_int(config,"others.interval",(long*)&interval) &&
	config_lookup_int(config,"others.mtu",(long*)&mtu) &&
	config_lookup_string(config,"others.user",&user) &&
	config_lookup_string(config,"others.pidfile",&pidfile))) {
		fprintf(stderr,"couldn't read miscoptions from config file\n");
		exit(EXIT_FAILURE);
	}
}
