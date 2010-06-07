#include <stdlib.h>
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

	/* decode basic protocol details */
	memcpy(&connection_id,msg,8);
	memcpy(&action,msg+8,4);
	memcpy(&transaction_id,msg+12,4);

	/* convert data to host order */
	#ifdef LITTLE_ENDIAN
	connection_id = bswap_64(connection_id);
	action = bswap_32(action);
	transaction_id = bswap_32(transaction_id);
	#endif

	/* call appropriate protocol handler */
	switch(action) {
		case 0:
			printf("CONNECT REQUEST\n");
			sbuffer = connect_request(&sbufLen,connection_id,transaction_id);
			break;
		case 1:
			printf("ANNOUNCE IPv4\n");
			sbuffer = announce4(&sbufLen,connection_id,transaction_id,msg);
			break;
		case 2:
			printf("SCRAPE\n");
			sbuffer = scrape(&sbufLen,connection_id,transaction_id,msg,msg_Len);
			break;
		case 4:
			printf("ANNOUNCE IPv6\n");
			sbuffer = announce6(&sbufLen,connection_id,transaction_id,msg);
			break;
		default:
			sbuffer = errormsg(&sbufLen,transaction_id,"Unkown action");
	}
	/* send resulting buffer */
	if(sendto(sock,sbuffer,sbufLen,0,cliAddr,cliLen) == -1) {
		LOGMSG(LOG_ERR,"Couldn't send Data");
	}
	/* free all allocated resources */
	free(sbuffer);
}
