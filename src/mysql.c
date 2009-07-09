#include <mysql.h>
#include <stdio.h>
#include <stdint.h>
#include <syslog.h>
#include <string.h>
#include <stdlib.h>

#include "database.h"

MYSQL* conn;

void connect_database(char *server,int port,char *user, char* password, char *database) {

        conn = mysql_init(NULL);
	
        /* Connect to database */
        if (!mysql_real_connect(conn, server,user, password, database,0, NULL, 0)) {
                syslog(LOG_ERR,"%s",mysql_error(conn));
                exit(1);
        }
        if(conn != NULL) {
                syslog(LOG_INFO,"Database Connection established");
        }
}

void disconnect_database() {

	mysql_close(conn);
}

int has_peer_announced_in_past(const char* infohash, const char* peerid) {

	int announced;
	char query[500];
	MYSQL_RES *res;
	MYSQL_ROW row;

	sprintf(query,"SELECT COUNT(*) FROM peers WHERE infohash='%s' AND peerid='%s'",infohash,peerid);
        syslog(LOG_DEBUG,"%s",query);
        if (mysql_query(conn, query)) {
                syslog(LOG_ERR,"%s",mysql_error(conn));
                exit(1);
        }   
        res = mysql_use_result(conn);
        row = mysql_fetch_row(res);
	announced=atoi(row[0]);
	mysql_free_result(res);	

	return announced;
}

void update_database4(const char* infohash,const char* peerid,uint64_t downloaded,uint64_t uploaded, uint64_t left, uint32_t ip, uint32_t key, uint16_t port,int announced) {

	char query[500];

	 if(announced==1) {
                sprintf(query,"UPDATE peers SET ts=NOW(),downloaded=%lld,bleft=%lld,uploaded=%lld,ipv4=%u,idkey=%u,port=%d WHERE infohash='%s' and peerid='%s'",downloaded,left,uploaded,ip,key,port,infohash,peerid);
        }
        //or insert the new infos if peer hasn't announced until now
        else {
        sprintf(query,"INSERT INTO peers VALUES(CURRENT_TIMESTAMP,'%s','%s',%lld,%lld,%lld,%u,'',%u,%d)",infohash,peerid,downloaded,left,uploaded,ip,key,port);
        }
        //execute query
        syslog(LOG_DEBUG,"%s",query);
        if (mysql_query(conn, query)) {
        	syslog(LOG_ERR,"%s",mysql_error(conn));
        	exit(1);
	}
}	

void update_database6(const char* infohash,const char* peerid,uint64_t downloaded,uint64_t uploaded, uint64_t left, char* ip, uint32_t key, uint16_t port,int announced) {

	char query[500];
	realloc(ip,17);
	*(ip+16) = '\0';

	 if(announced==1) {
                sprintf(query,"UPDATE peers SET ts=NOW(),downloaded=%lld,bleft=%lld,uploaded=%lld,ipv6=%s,idkey=%u,port=%d WHERE infohash='%s' and peerid='%s'",downloaded,left,uploaded,ip,key,port,infohash,peerid);
        }
        //or insert the new infos if peer hasn't announced until now
        else {
        sprintf(query,"INSERT INTO peers VALUES(CURRENT_TIMESTAMP,'%s','%s',%lld,%lld,%lld,0,'%s',%u,%d)",infohash,peerid,downloaded,left,uploaded,ip,key,port);
        }
        //execute query
        syslog(LOG_DEBUG,"%s",query);
        if (mysql_query(conn, query)) {
        	syslog(LOG_ERR,"%s",mysql_error(conn));
        	exit(1);
	}
}	

void delete_old_entries(int interval) {

	char query[500];

	sprintf(query,"DELETE FROM peers WHERE TIMESTAMPDIFF(SECOND,ts,NOW())>%u",2*interval);
        syslog(LOG_DEBUG,"%s",query);
        if (mysql_query(conn, query)) {
                syslog(LOG_ERR,"%s",mysql_error(conn));
                exit(1);
        }
}

void seeders_and_leechers(const char* infohash,int* seeders, int* leechers) {

	char query[500];
	MYSQL_RES *res;
	MYSQL_ROW row;

	 sprintf(query,"SELECT COUNT(*) AS 'Seeders',(SELECT COUNT(*) FROM peers WHERE bleft!=0 AND infohash='%s' AND ipv4!=0) AS 'Leechers' FROM peers WHERE bleft=0 AND infohash='%s' AND ipv4!=0",infohash,infohash);
        syslog(LOG_DEBUG,"%s",query);
        if (mysql_query(conn,query)) {
		syslog(LOG_ERR,"%s",mysql_error(conn));
                exit(1);
        }
        res = mysql_use_result(conn);
        row = mysql_fetch_row(res);

        *seeders = atoi(row[0]);
        *leechers = atoi(row[1]);

        mysql_free_result(res);
}

Peer* get_peer_data4(int *peerLen,const char* infohash,int number_of_entries) {

	MYSQL_RES *res;
	MYSQL_ROW row;
	char query[500];
	int num_peers;

	//TODO: check if num_want isn't larger than the the max possible (Datagram can't be bigger than MTU)
	sprintf(query,"SELECT ipv4,port FROM peers WHERE infohash='%s' AND ipv4!=0 ORDER BY RAND() LIMIT %u",infohash,number_of_entries);
        syslog(LOG_DEBUG,"%s",query);
        if (mysql_query(conn, query)) {
                syslog(LOG_ERR,"%s",mysql_error(conn));
                exit(1);
        }
        res = mysql_store_result(conn);
	num_peers = mysql_num_rows(res);
	*peerLen = num_peers;
	
	Peer *peers = malloc(sizeof(Peer)*num_peers);
	
	int i;
	for(i=0;i<num_peers;i++) {

		row = mysql_fetch_row(res);
		peers[i].ipv4 = atoi(row[0]);
		peers[i].port = atoi(row[1]);
	}
	
	return peers;
}

Peer* get_peer_data6(int *peerLen,const char* infohash,int number_of_entries) {

	MYSQL_RES *res;
	MYSQL_ROW row;
	char query[500];
	int num_peers;

	//TODO: check if num_want isn't larger than the the max possible (Datagram can't be bigger than MTU)
	sprintf(query,"SELECT ipv6,port FROM peers WHERE infohash='%s' AND ipv4!=0 ORDER BY RAND() LIMIT %u",infohash,number_of_entries);
        syslog(LOG_DEBUG,"%s",query);
        if (mysql_query(conn, query)) {
                syslog(LOG_ERR,"%s",mysql_error(conn));
                exit(1);
        }
        res = mysql_store_result(conn);
	num_peers = mysql_num_rows(res);
	*peerLen = num_peers;
	
	Peer *peers = malloc(sizeof(Peer)*num_peers);
	
	int i;
	for(i=0;i<num_peers;i++) {

		row = mysql_fetch_row(res);
		memcpy(peers[i].ipv6,row[0],16);
		peers[i].port = atoi(row[1]);
	}
	
	return peers;
}

int is_user_authenticated(const char* username, const char* hash) {

	return 0;
}
