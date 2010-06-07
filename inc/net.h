#include <arpa/inet.h>
#include <netdb.h>
#include <stdint.h>

int *sockets;
int num_sockets;

struct bt_connect_request {
	int64_t connection_id;
	int32_t action;
	int32_t transaction_id;
};

struct bt_connect_reply {
	int32_t action;
	int32_t transaction_id;
	int64_t connection_id;
};

int bind4(const char* host, int port);
int bind6(const char* host, int port);
void check_readable_socket();
void send_receive(int sock);
