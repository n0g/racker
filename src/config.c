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
	int portnr, interface_num;

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
		printf("Interface %d\n\tType: %s\n\tHostname: %s\n\tPort: %d\n",interface_num,type,hostname,portnr);
	}
}

void config_other(config_t *config) {
	int mtu, interval;
	const char *user, *pidfile;

	if(config_lookup_int(config,"others.interval",(long*)&interval) &&
	config_lookup_int(config,"others.mtu",(long*)&mtu) &&
	config_lookup_string(config,"others.user",&user) &&
	config_lookup_string(config,"others.pidfile",&pidfile)) {
		printf("Interval: %d\nMaximum Transfer Unit: %d\nUsername: %s\nPID File: %s\n",interval,mtu,user,pidfile);
	}
}
