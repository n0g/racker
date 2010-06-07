#ifndef BT_NET
#define BT_NET
#include <arpa/inet.h>
#include <netdb.h>
#include <stdint.h>

int *sockets;
int num_sockets;

struct bt_peer4 {
        uint32_t ipv4;
        uint16_t port;
	struct bt_peer4 *next;
};

struct bt_connect_request {
	int64_t connection_id;
	int32_t action;
	int32_t transaction_id;
} __attribute__((__packed__));

struct bt_connect_reply {
	int32_t action;
	int32_t transaction_id;
	int64_t connection_id;
} __attribute__((__packed__));

struct bt_announce4_request {
	int64_t connection_id;
	int32_t action;
	int32_t transaction_id;
	char info_hash[20];
	char peer_id[20];
	int64_t downloaded;
	int64_t left;
	int64_t uploaded;
	int32_t event;
	uint32_t ip;
	uint32_t key;
	int32_t num_want;
	uint16_t port;
	uint16_t extensions;
} __attribute__((__packed__));

struct bt_announce4_reply {
	int32_t action;
	int32_t transaction_id;
	int32_t interval;
	int32_t leechers;
	int32_t seeders;
	struct bt_peer4 *list;
};

struct bt_error {
	int32_t action;
	int32_t transaction_id;
	char *message;
} __attribute__((__packed__));
	

int bind4(const char* host, int port);
int bind6(const char* host, int port);
void check_readable_socket();
void send_receive(int sock);

struct bt_connect_request* unpack_connect(int msgLen, char *msg);
char* pack_connect(int *msgLen, struct bt_connect_reply *reply);
#endif
