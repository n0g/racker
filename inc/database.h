#include <stdint.h>

int has_peer_announced_in_past(const char* infohash, const char* peerid);
void update_database4(const char* infohash,const char* peerid,uint64_t downloaded,uint64_t uploaded, uint64_t left, uint32_t ip, uint32_t key, uint16_t port,int announced);
void update_database6(const char* infohash,const char* peerid,uint64_t downloaded,uint64_t uploaded, uint64_t left, char* ip, uint32_t key, uint16_t port,int announced);
void delete_old_entries(int interval);
void seeders_and_leechers(const char* infohash,int* seeders, int* leechers);
/* (struct bt_peer4*) get_peer_data4(int *peerLen, const char* infohash,int number_of_entries);
Peer* get_peer_data6(int *peerLen, const char* infohash,int number_of_entries); */
int is_user_authenticated(const char* username, const char* hash);
