#include <string.h>
#include <syslog.h>
#include <stdlib.h>
#include <stdio.h>

#include "utils.h"
#include "net.h"

uint64_t generate_connection_id() {

        uint64_t connection_id;
        uint32_t tmp;

        srand((unsigned) time(NULL));
        tmp = rand();
        memcpy(&connection_id,&tmp,4);
        tmp = rand();
        memcpy(((char*)(&connection_id))+4,&tmp,4);

        return connection_id;
}

/*
char* strToHexStr(char* str, int len) {
	//TODO: remove dependancy on mysql library
        char *newstr = malloc(len*2+1);
        mysql_hex_string(newstr,str,len);
        return(newstr);
}
*/

char* strToHexStr(char *str,int len)
{
	char *hex = malloc(len*2+1);
	char *rtr = hex;

	while(len--) {
		sprintf(hex, "%.2X",(char)*str++);
		hex+=2;
	}
	*hex = '\0';
	return rtr;
} 

void sig_handler(int signal) {

	syslog(LOG_INFO,"----- racker is stopping -----");
	closelog();

	int i;
	for(i=0;i<num_sockets;i++)
		shutdown(sockets[i],2);
}

void write_pid_file(char* filename, int pid) {

        FILE *file;
        file = fopen(filename,"w");
        fprintf(file,"%u",pid);
        fclose(file);
        syslog(LOG_DEBUG,"Wrote PID File to %s",filename);
}

void drop_root_rights(char* username) {

        struct passwd *user;
        user = getpwnam(username);
        setuid(user->pw_uid);
        syslog(LOG_DEBUG,"Changed executing User to %s",username);
}

void daemonize() {

        int pid;
        pid = fork();
        if (pid > 0) {
                syslog(LOG_INFO,"Exiting Parent");
                exit(EXIT_SUCCESS);
        }
}
