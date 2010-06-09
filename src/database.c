#include <stdio.h>
#include <stdint.h>
#include <syslog.h>
#include <string.h>
#include <stdlib.h>

#include "database.h"
#include "bintree.h"

int has_peer_announced_in_past(const char* infohash, const char* peerid) {

#if 0
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
#endif
}

void update_database(struct bt_peer *peer) {

#if 0
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
#endif
}	

void delete_old_entries(int interval) {

#if 0
	char query[500];

	sprintf(query,"DELETE FROM peers WHERE TIMESTAMPDIFF(SECOND,ts,NOW())>%u",2*interval);
        syslog(LOG_DEBUG,"%s",query);
        if (mysql_query(conn, query)) {
                syslog(LOG_ERR,"%s",mysql_error(conn));
                exit(1);
        }
#endif
}

void seeders_and_leechers(const char* infohash,int* seeders, int* leechers) {

#if 0
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
#endif
}

struct bt_peer* get_peer_data(int ip_version,const char* infohash,int number_of_entries) {

#if 0
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
#endif
}

int is_user_authenticated(const char* username, const char* hash) {
	return 0;
}
