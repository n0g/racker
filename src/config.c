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
	int port, interface_num = 0;

	/* read interface settings */
	if((interfaces = config_lookup(config,"interfaces")) == NULL) {
		fprintf(stderr,"Couldn't read Interfaces\n");
	}

	while((interface = config_setting_get_elem(interfaces,interface_num)) != NULL) {
		if(!(config_setting_lookup_string(interface,"type",&type) &&
		config_setting_lookup_string(interface,"hostname",&hostname) &&
		config_setting_lookup_int(interface,"port",&port))) {
			continue;
		}
		
		interface_num++;
	}
}

void config_other(char *filename) {
	
}

int main(int argc, char *argv[]) {
	config_listeners(argv[1]);
	return EXIT_SUCCESS;
}
