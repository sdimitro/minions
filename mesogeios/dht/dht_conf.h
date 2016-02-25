#ifndef __DHT_CONF_H
#define __DHT_CONF_H

#define CONFIG_FILENAME "dht.conf"

typedef struct {
  int port;
  char *host;
} peer_t;

typedef struct {
  peer_t *list;
  int size;
} peer_list_t;

peer_list_t *alloc_peer_list(char *buf);
peer_list_t *alloc_peer_state();
void free_peer_state(peer_list_t *pl);
int locate_dpeer(char *key, peer_list_t *pl);
int locate_drep_peer(char *key, peer_list_t *pl, int repn);

#endif
