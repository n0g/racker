#include <byteswap.h>
#include <endian.h>
#include <stdint.h>
#include <netinet/in.h>

#include "net.h"

struct bt_connect_reply* connect_request(struct bt_connect_request *request);
struct bt_announce4_reply* announce4(struct bt_announce4_request *request);
char* scrape(int* sbufLen,uint64_t connection_id, uint32_t transaction_id, char msg[],int msgLen);
char* announce6(int* sbufLen,struct sockaddr_in6 cliAddr,uint64_t connection_id, uint32_t transaction_id, char msg[]);
char* errormsg(int* sbufLen,uint32_t transaction_id,char* error_msg);
