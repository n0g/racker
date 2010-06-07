#include <syslog.h>
#include <stdlib.h>
#include <string.h>
#include <libconfig.h>

#include "actions.h"
#include "database.h"
#include "utils.h"
#include "config.h"
#include "net.h"

struct bt_connect_reply* connect_request(struct bt_connect_request *request) {

	struct bt_connect_reply *reply = malloc(sizeof(struct bt_connect_reply));
	reply->action = 0;	
	reply->connection_id = generate_connection_id();
	reply->transaction_id = request->transaction_id;
	
	/* will be 0x41727101980 (network order) on connect 
	4497486125440 in decimal */
	if( request->connection_id != 4497486125440LLU ) {
		LOGMSG(LOG_INFO,"Client sent wrong connection_id on first connect");
		/* 
		sbuffer = errormsg(sbufLen,transaction_id, "Wrong connection_id on first connect.");
		return sbuffer;
		*/
	}

	LOGMSG(LOG_DEBUG,"Connection id: %lld",request->connection_id);
	/* TODO: save this generated connection id + ip in database */
	LOGMSG(LOG_DEBUG,"Transaction id: %u",reply->transaction_id);
	LOGMSG(LOG_DEBUG,"generated Connection ID: %lld",reply->connection_id);

	return reply; 
}

char* announce4(int* sbufLen,struct sockaddr_in cliAddr,uint64_t connection_id, uint32_t transaction_id, const char *msg) {
	
	/* TODO: check if connection_id is known */

	char info_hash[20], peer_id[20];
	char *info_hash_hex, *peer_id_hex;
	int64_t downloaded,left,uploaded;
	int32_t event,num_want,num_peers,action;
	uint32_t ip, key, seeders, leechers;	
	uint16_t port, extensions;

	memcpy(&info_hash,msg+16,20);
	memcpy(&peer_id,msg+36,20);
	memcpy(&downloaded,msg+56,8);
	memcpy(&left,msg+64,8);
	memcpy(&uploaded,msg+72,8);
	memcpy(&event,msg+80,4);
	memcpy(&ip,msg+84,4);
	memcpy(&key,msg+88,4);
	memcpy(&num_want,msg+92,4);
	memcpy(&port,msg+96,2);
	memcpy(&extensions,msg+98,2);
	
	/* replace 0 (default) ip with the real ip address */
	if(ip==0) {
		ip = cliAddr.sin_addr.s_addr;		
	}

	#ifdef LITTLE_ENDIAN
	downloaded = bswap_64(downloaded);
	left = bswap_64(left);
	uploaded = bswap_64(uploaded);
	event = bswap_32(event);
	ip = bswap_32(ip);
	key = bswap_32(key);
	num_want = bswap_32(num_want);
	port = bswap_16(port);
	extensions = bswap_16(extensions);
	#endif
	
	/* convert info_hash and peer_id to hex so that they are printable */
	info_hash_hex = strToHexStr(info_hash,20);
	peer_id_hex = strToHexStr(peer_id,20);

	LOGMSG(LOG_DEBUG,"Connection ID: %lld",connection_id);
	LOGMSG(LOG_DEBUG,"Info Hash: %s Peer ID: %s",info_hash_hex, peer_id_hex);
	LOGMSG(LOG_DEBUG,"Downloaded: %lld Left: %lld Uploaded: %lld",downloaded,left,uploaded);
	LOGMSG(LOG_DEBUG,"Event: %uNumber of Peers wanted: %u",event, num_want);

	/* TODO: if authentication extension is activated check if auth correct */
	
	#if 0
	/* check if peer has already announced */
	int announced = has_peer_announced_in_past(info_hash_hex,peer_id_hex);
        /* update entry if peer has announced already */
	update_database4(info_hash_hex,peer_id_hex,downloaded,uploaded,left,ip,key,port,announced);
	/* delet old peers from database */
	delete_old_entries(interval);
        /* Get number of seeders and leechers */
	seeders_and_leechers(info_hash_hex,&seeders,&leechers);
	/* get data of the peers from the database */
	Peer *peers;
	peers = get_peer_data4(&num_peers,info_hash_hex,num_want);


	*sbufLen = 20+num_peers*6;
	char* sbuffer = malloc(*sbufLen);
	action = 1;
	
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

	LOGMSG(LOG_DEBUG,"Number of Peers available: %u",num_peers);
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

	return sbuffer;
	#endif
	return NULL;
}

char* scrape(int* sbufLen,uint64_t connection_id, uint32_t transaction_id, char msg[],int msgLen) {
	
	/* TODO: check if connection_id and transaction_id is known */
	int action = 2;
	int request_num;
	request_num =(msgLen-16)/20;
	LOGMSG(LOG_DEBUG,"Number of Requested Hashes: %u",request_num);
	*sbufLen = 8+request_num*12;
	char* sbuffer = malloc(*sbufLen);
 	
	#ifdef LITTLE_ENDIAN
	action = bswap_32(action);
	transaction_id = bswap_32(transaction_id);
	#endif
	
	memcpy(sbuffer,&action,4);
	memcpy(sbuffer+4,&transaction_id,4);

	#if 0
	while(request_num--) {
		char info_hash[20];
		char* info_hash_hex;

		memcpy(info_hash,msg+16+request_num*20,20);
		info_hash_hex = strToHexStr(info_hash,20);
		LOGMSG(LOG_DEBUG," Info-Hash: %s",info_hash_hex);
		int complete,downloaded,incomplete;

	
		/* TODO: number of total downloads is missing
		EVENT "complete" in announce has to be saved */
		seeders_and_leechers(info_hash_hex,&downloaded,&incomplete);
		complete = 0;	
		LOGMSG(LOG_DEBUG," Down: %u Incomp: %u",downloaded, incomplete);

		#ifdef LITTLE_ENDIAN
		complete = bswap_32(complete);
		downloaded = bswap_32(downloaded);
		incomplete = bswap_32(incomplete);
		#endif

		memcpy(sbuffer+8+request_num*12,&complete,4);
		memcpy(sbuffer+12+request_num*12,&downloaded,4);
		memcpy(sbuffer+16+request_num*12,&incomplete,4);
	}	
	
	return sbuffer;	
	#endif
	return NULL;
}
char* announce6(int* sbufLen,struct sockaddr_in6 cliAddr,uint64_t connection_id, uint32_t transaction_id, char msg[]) {
	
	/* TODO: check if connection_id is known */

	char info_hash[20], peer_id[20];
	char *info_hash_hex, *peer_id_hex,*ip=malloc(16);
	int64_t downloaded,left,uploaded;
	int32_t event,num_want,num_peers,action;
	uint32_t key, seeders, leechers;	
	uint16_t port, extensions;

	memcpy(&info_hash,msg+16,20);
	memcpy(&peer_id,msg+36,20);
	memcpy(&downloaded,msg+56,8);
	memcpy(&left,msg+64,8);
	memcpy(&uploaded,msg+72,8);
	memcpy(&event,msg+80,4);
	memcpy(&ip,msg+84,16);
	memcpy(&key,msg+100,4);
	memcpy(&num_want,msg+104,4);
	memcpy(&port,msg+108,2);
	memcpy(&extensions,msg+110,2);
	
	
	/* replace 0 (default) ip with the real ip address */
	if(strcmp(ip,"")==0) {
		ip = cliAddr.sin6_addr.s6_addr;		
	}

	#ifdef LITTLE_ENDIAN
	downloaded = bswap_64(downloaded);
	left = bswap_64(left);
	uploaded = bswap_64(uploaded);
	event = bswap_32(event);
	key = bswap_32(key);
	num_want = bswap_32(num_want);
	port = bswap_16(port);
	extensions = bswap_16(extensions);
	#endif
	
	#if 0
	/* convert info_hash and peer_id to hex so that they are printable */
	info_hash_hex = strToHexStr(info_hash,20);
	peer_id_hex = strToHexStr(peer_id,20);

	LOGMSG(LOG_DEBUG,"Connection ID: %lld",connection_id);
	LOGMSG(LOG_DEBUG,"Info Hash: %sPeer ID: %s",info_hash_hex, peer_id_hex);
	LOGMSG(LOG_DEBUG,"Downloaded: %lldLeft: %lldUploaded: %lld",downloaded,left,uploaded);
	LOGMSG(LOG_DEBUG,"Event: %uNumber of Peers wanted: %u",event, num_want);

	/* TODO: if authentication extension is activated check if auth correct */
	
	/* check if peer has already announced */
	int announced = has_peer_announced_in_past(info_hash_hex,peer_id_hex);
        /* update entry if peer has announced already */
	update_database6(info_hash_hex,peer_id_hex,downloaded,uploaded,left,ip,key,port,announced);
	/* delete old peers from database */
	delete_old_entries(interval);
        /* Get number of seeders and leechers */
	seeders_and_leechers(info_hash_hex,&seeders,&leechers);
	/* get data of the peers from the database */
	Peer *peers;
	peers = (Peer*) get_peer_data6(&num_peers,info_hash_hex,num_want);


	*sbufLen = 20+num_peers*18;
	char* sbuffer = malloc(*sbufLen);
	action = 1;
	
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

	LOGMSG(LOG_DEBUG,"Number of Peers available: %u",num_peers);
	while(num_peers--) {
		ip = peers[num_peers].ipv6; 
		port = peers[num_peers].port; 

		#ifdef LITTLE_ENDIAN
		port = bswap_16(port);
		#endif
		
		memcpy(sbuffer+20+num_peers*6,&ip,16);
		memcpy(sbuffer+36+num_peers*6,&port,2);
	}

	return sbuffer;
	#endif
	return NULL;
}

char* errormsg(int* sbufLen,uint32_t transaction_id,char* error_msg) {

	int32_t action;
	int msgLen;

	action=3;
	msgLen = strlen(error_msg);
	*sbufLen = msgLen+8;
	char* sbuffer = malloc(*sbufLen);

	/* convert them to network order */
	#ifdef LITTLE_ENDIAN
	action = bswap_32(action);
	transaction_id = bswap_32(transaction_id);
	#endif		

	memcpy(sbuffer,&action,4);
	memcpy(sbuffer+4,&transaction_id,4);
	memcpy(sbuffer+8,&error_msg,msgLen);
	
	LOGMSG(LOG_INFO,"ERRORTransaction ID: %u Text: %s",transaction_id,error_msg);
	
	return sbuffer;
}
