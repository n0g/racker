#include <stdlib.h>
#include <stdint.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>
#include <byteswap.h>
#include <endian.h>
#include <libconfig.h>
#include <sys/select.h>

#include "net.h"
#include "config.h"
#include "utils.h"

int bind4(const char* host, int port) {
	struct sockaddr_in ip4addr;
	int s;

	ip4addr.sin_family = AF_INET;
	ip4addr.sin_port = htons(port);
	inet_pton(AF_INET, host, &ip4addr.sin_addr);

	s = socket(PF_INET, SOCK_DGRAM, 0);
	if(bind(s, (struct sockaddr*)&ip4addr, sizeof ip4addr) == -1) {
		LOGMSG(LOG_ERR,"couldn't bind to interface");
	}
	
	return s;
}

int bind6(const char* host, int port) {
	struct sockaddr_in6 ip6addr;
	int s;

	ip6addr.sin6_family = AF_INET6;
	ip6addr.sin6_port = htons(port);
	inet_pton(AF_INET6, host, &ip6addr.sin6_addr);

	s = socket(PF_INET6, SOCK_DGRAM, 0);
	if(bind(s, (struct sockaddr*)&ip6addr, sizeof ip6addr) == -1) {
		LOGMSG(LOG_ERR,"couldn't bind to interface");
	}
	
	return s;
}

void check_readable_socket() {
	fd_set fds;
	int numfds, maxfd;

	while(1) {
		FD_ZERO(&fds);	
		numfds = num_sockets;
		maxfd = 0;
		/* fill fd_set */
		while(numfds--) {
			FD_SET(sockets[numfds],&fds);
			if(sockets[numfds] > maxfd) {
				maxfd = sockets[numfds];
			}
		}
		/* wait until at least one fd becomes readable */
		select(maxfd+1,&fds,NULL,NULL,NULL);
		/* handle clients */
		numfds = num_sockets;
		while(numfds--) {
			if(FD_ISSET(sockets[numfds],&fds)) {
				send_receive(sockets[numfds]);
			}
		}
	}
}

void send_receive(int sock) {
	char msg[mtu], *sbuffer;	
	struct sockaddr_in cliAddr;
	int cliLen, msgLen, sbufLen;

	uint64_t connection_id;
	int32_t action;
	uint32_t transaction_id;

	/* receive data from socket */
	memset(msg,0x0,mtu);
	errno = 0;
	cliLen = sizeof(struct sockaddr_in);
	msgLen = recvfrom(sock, msg, mtu, 0, (struct sockaddr *) &cliAddr, &cliLen);
	if(msgLen == -1) {
		perror("couldn't receive data");
	}	
	LOGMSG(LOG_DEBUG,"Received %d Bytes of Data\n",msgLen);
	
	struct bt_connect_request *request = unpack_connect(msgLen,msg);
	/* call appropriate protocol handler */
	switch(request->action) {
		case 0:
			printf("CONNECT REQUEST\n");
			struct bt_connect_reply *reply_connect = (struct bt_connect_reply*) connect_request(request);
			sbuffer = (char*) pack_connect(&sbufLen,reply_connect);
			free(reply_connect);
			break;
		case 1:
			printf("ANNOUNCE IPv4\n");
			struct bt_announce4_request *request_announce4 = (struct bt_announce4_request*) unpack_announce4(msgLen,msg);
			struct bt_announce4_reply *reply_announce4 = (struct bt_announce4_reply*) announce4(request_announce4);
			sbuffer = (char*) pack_announce4(&sbufLen, reply_announce4);
			free(request_announce4);
			free(reply_announce4);
			break;
		case 2:
			printf("SCRAPE\n");
			sbuffer = (char*)scrape(&sbufLen,request->connection_id,request->transaction_id,msg,msgLen);
			break;
		case 4:
			printf("ANNOUNCE IPv6\n");
			sbuffer = (char*)announce6(&sbufLen,request->connection_id,request->transaction_id,msg);
			break;
		default:
			sbuffer = (char*)errormsg(&sbufLen,request->transaction_id,"Unkown action");
	}
	/* send resulting buffer */
	if(sendto(sock,sbuffer,sbufLen,0,(struct sockaddr*) &cliAddr,(socklen_t) cliLen) == -1) {
		LOGMSG(LOG_ERR,"Couldn't send Data");
	}
	/* free all allocated resources */
	free(sbuffer);
	free(request);
}

struct bt_connect_request* unpack_connect(int msgLen,char *msg) {
	struct bt_connect_request *request = malloc(sizeof(struct bt_connect_request));
	memcpy(request,msg,16);
	/* convert data to host order */
	#ifdef LITTLE_ENDIAN
	request->connection_id = bswap_64(request->connection_id);
	request->action = bswap_32(request->action);
	request->transaction_id = bswap_32(request->transaction_id);
	#endif

	return request;	
}

char* pack_connect(int *msgLen,struct bt_connect_reply *reply) {
	*msgLen = sizeof(struct bt_connect_reply);
	char *sbuffer = malloc(*msgLen);

        #ifdef LITTLE_ENDIAN
        reply->connection_id = bswap_64(reply->connection_id);
        reply->action = bswap_32(reply->action);
        reply->transaction_id = bswap_32(reply->transaction_id);
        #endif

	memcpy(sbuffer,reply,16);
	return sbuffer;
}

struct bt_announce4_request* unpack_announce4(int msgLen, char *msg) {
	int requestLen = sizeof(struct bt_announce4_request);
	struct bt_announce4_request *request = malloc(requestLen);
	memcpy(request,msg,requestLen);	
	
	#ifdef LITTLE_ENDIAN
        request->downloaded = bswap_64(request->downloaded);
        request->left = bswap_64(request->left);
        request->uploaded = bswap_64(request->uploaded);
        request->event = bswap_32(request->event);
        request->ip = bswap_32(request->ip);
        request->key = bswap_32(request->key);
        request->num_want = bswap_32(request->num_want);
        request->port = bswap_16(request->port);
        request->extensions = bswap_16(request->extensions);
        #endif

	return request;
}

char* pack_announce4(int *msgLen, struct bt_announce4_reply *reply) {
	/*
        #ifdef LITTLE_ENDIAN
        action = bswap_32(action);
        interval = bswap_32(interval);
        transaction_id = bswap_32(transaction_id);
        seeders = bswap_32(seeders);
        leechers = bswap_32(leechers);  
        #endif

        memcpy(sbuffer,&action,4);
        memcpy(sbuffer+4,&transaction_id,4);
        memcpy(sbuffer+8,&interval,4);
        memcpy(sbuffer+12,&leechers,4);
        memcpy(sbuffer+16,&seeders,4);

        while(num_peers--) {
                ip = peers[num_peers].ipv4; 
                port = peers[num_peers].port; 

                LOGMSG(LOG_DEBUG,"%u %u",ip,port);
                        
                #ifdef LITTLE_ENDIAN
                ip = bswap_32(ip);
                port = bswap_16(port);
                #endif
                
                memcpy(sbuffer+20+num_peers*6,&ip,4);
                memcpy(sbuffer+24+num_peers*6,&port,2);
        }
	*/
	return NULL;

}
