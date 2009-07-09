#include <syslog.h>
#include <string.h>
#include <byteswap.h>
#include <endian.h>

#include "net.h"
#include "config.h"

int bind4(const char* host, int port) {

	struct sockaddr_in ip4addr;
	int s;

	ip4addr.sin_family = AF_INET;
	ip4addr.sin_port = htons(port);
	inet_pton(AF_INET, host, &ip4addr.sin_addr);

	s = socket(PF_INET, SOCK_DGRAM, 0);
	bind(s, (struct sockaddr*)&ip4addr, sizeof ip4addr);
	
	return s;
}

int bind6(const char* host, int port) {

	struct sockaddr_in6 ip6addr;
	int s;

	ip6addr.sin6_family = AF_INET6;
	ip6addr.sin6_port = htons(port);
	inet_pton(AF_INET6, host, &ip6addr.sin6_addr);

	s = socket(PF_INET6, SOCK_DGRAM, 0);
	bind(s, (struct sockaddr*)&ip6addr, sizeof ip6addr);
	
	return s;
}

void send_receive_loop4(void *s) {

	char msg[mtu], *sbuffer;	
	struct sockaddr_in cliAddr;
	int cliLen, msgLen,sbufLen,socket;

	socket = (int) s;

	while(1) { 
		memset(msg,0x0,mtu);

		cliLen = sizeof(cliAddr);
		msgLen = recvfrom(socket, msg, mtu, 0, (struct sockaddr *) &cliAddr, &cliLen);

		if(msgLen<=0) {
			syslog(LOG_ERR,"cannot receive data");
			return;
		}
      
		uint64_t connection_id;
		int32_t action;
		uint32_t transaction_id;

		//print senders address and port
		syslog(LOG_DEBUG,"New Packet:Client IP: %s", inet_ntoa(cliAddr.sin_addr)); 
		syslog(LOG_DEBUG,"Client Port: %hu", ntohs(cliAddr.sin_port));
		//get basic protocol details from client
		memcpy(&connection_id,msg,8);
		memcpy(&action,msg+8,4);
		memcpy(&transaction_id,msg+12,4);
	
		//convert them to host order
		#ifdef LITTLE_ENDIAN
		connection_id = bswap_64(connection_id);
		action = bswap_32(action);
		transaction_id = bswap_32(transaction_id);
		#endif

		syslog(LOG_DEBUG,"Datagram length: %u",msgLen);
		switch( action ) {

			case 0: syslog(LOG_INFO, "CONNECT");
				sbuffer = (char *)connect_request(&sbufLen,connection_id,transaction_id);
				break;

			case 1: syslog(LOG_INFO, "IPv4 ANNOUNCE");
				sbuffer = (char *)announce4(&sbufLen,cliAddr,connection_id,transaction_id,msg);
				break;

			case 2: syslog(LOG_INFO, "SCRAPE");
				sbuffer = (char *)scrape(&sbufLen,connection_id,transaction_id,msg,msgLen);
				break;

			default: syslog(LOG_ERR,"Didn't recognize Action %u",action);

		}
		sendto(socket,sbuffer,sbufLen,0,(struct sockaddr *) &cliAddr,sizeof(cliAddr));
		free(sbuffer);
	}
}

void send_receive_loop6(void *s) {

	char msg[mtu], *sbuffer;	
	struct sockaddr_in6 cliAddr;
	int cliLen, msgLen,sbufLen, socket;
	socket = (int) s;

	while(1) { 
		memset(msg,0x0,mtu);

		cliLen = sizeof(cliAddr);
		msgLen = recvfrom(socket, msg, mtu, 0, (struct sockaddr *) &cliAddr, &cliLen);

		if(msgLen<=0) {
			syslog(LOG_ERR,"cannot receive data");
			return;
		}
      
		uint64_t connection_id;
		int32_t action;
		uint32_t transaction_id;

		//print senders address and port
		//syslog(LOG_DEBUG,"New Packet:Client IP: %s", inet_pntoa(cliAddr.sin6_addr)); 
		//syslog(LOG_DEBUG,"Client Port: %hu", ntohs(cliAddr.sin6_port));
		//get basic protocol details from client
		memcpy(&connection_id,msg,8);
		memcpy(&action,msg+8,4);
		memcpy(&transaction_id,msg+12,4);
	
		//convert them to host order
		#ifdef LITTLE_ENDIAN
		connection_id = bswap_64(connection_id);
		action = bswap_32(action);
		transaction_id = bswap_32(transaction_id);
		#endif

		syslog(LOG_DEBUG,"Datagram length: %u",msgLen);
		switch( action ) {

			case 0: syslog(LOG_INFO, "CONNECT");
				sbuffer = (char *)connect_request(&sbufLen,connection_id,transaction_id);
				break;

			case 1: syslog(LOG_INFO, "IPv6 ANNOUNCE");
				sbuffer = (char *)announce6(&sbufLen,cliAddr,connection_id,transaction_id,msg);
				break;

			case 2: syslog(LOG_INFO, "SCRAPE");
				sbuffer = (char *)scrape(&sbufLen,connection_id,transaction_id,msg,msgLen);
				break;

			//someone specified action 4 as ipv6 announce 
			case 4: syslog(LOG_INFO, "IPv6 ANNOUNCE");
				sbuffer = (char *)announce6(&sbufLen,cliAddr,connection_id,transaction_id,msg);
				break;

			default: syslog(LOG_ERR,"Didn't recognize Action %u",action);

		}
		
		sendto(socket,sbuffer,sbufLen,0,(struct sockaddr *) &cliAddr,sizeof(cliAddr));
		free(sbuffer);
	}
}
