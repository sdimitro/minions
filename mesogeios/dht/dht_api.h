#ifndef __DHT_API_H
#define __DHT_API_H

#include "dht_conf.h"

int dht_put(peer_list_t *pl, char *key, char *val);
int dht_get(peer_list_t *pl, char *key, char *val);
int dht_del(peer_list_t *pl, char *key, char *val);

#endif
