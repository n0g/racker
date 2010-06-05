int mtu, interval;
char *pidfile, *user;

config_t* config_initialize(char *filename);
void config_listeners(config_t *config);
void config_other(config_t *config);
