#include <arpa/inet.h>
#include <netdb.h>

int *sockets;
int num_sockets;

int bind4(const char* host, int port);
int bind6(const char* host, int port);
void send_receive_loop();
