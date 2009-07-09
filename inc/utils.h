#include <time.h>
#include <pwd.h>
#include <stdint.h>

uint64_t generate_connection_id();
char* strToHexStr(char* str, int len);
void sig_handler(int signal);
void write_pid_file(char* filename, int pid);
void drop_root_rights(char* username);
void daemonize();
