#include <stdint.h>

struct bt_peer {
	char peerid[20];
	uint32_t key;
	uint64_t downloaded;
	uint64_t left;
	uint64_t uploaded;	
	uint32_t ip4;
	char ipv6[16];
	uint16_t port;
	struct bt_peer *next;
};
struct bt_torrent {
	char info_hash[20];
	uint32_t seeders;
	uint32_t leechers;
	struct bt_peer *next;
};

void initialize_database();
int compare(const void *data1, const void *data2);
int has_peer_announced_in_past(const char* infohash, const char* peerid);
void update_database(struct bt_peer *peer);
void delete_old_entries(int interval);
void seeders_and_leechers(const char* infohash,int* seeders, int* leechers);
struct bt_peer* get_peer_data(int ip_version,const char* infohash,int number_of_entries);
int is_user_authenticated(const char* username, const char* hash);
