#include <stdio.h>
#include <stdint.h>
#include <syslog.h>
#include <string.h>
#include <stdlib.h>

#include "database.h"
#include "bintree.h"

struct bt_node *tree;

void initialize_database() {
	tree = NULL;
}

int compare(const void* data1, const void *data2) {
	char *a = ((struct bt_torrent*)data1)->info_hash;
	char *b = ((struct bt_torrent*)data2)->info_hash;
	int i;
	for(i=0;i<20;i++) {
		if(a[i] > b[i]) {
			return -1;
		} else if(a[i] < b[i]) {
			return 1;
		}
	}
	return 0;
}

int has_peer_announced_in_past(const char* infohash, const char* peerid) {
	struct bt_torrent tmp;
	tmp.info_hash = infohash;

	struct bt_node **node = search(&tree,&compare, &tmp);
	if(*node != NULL) {
		while((*node)->data->next != NULL) {
			/* walk through all peers and check if the peerid exists */
		}	
	}
	return 0;
}

void update_database(struct bt_peer *peer) {
	
}	

void delete_old_entries(int interval) {

}

void seeders_and_leechers(const char* infohash,int* seeders, int* leechers) {
	/* search for torrent */
	/* if it exists set seeders and leechers appropriately */
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
