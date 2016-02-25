#ifndef __HT_H
#define __HT_H

#include <stdlib.h>

typedef struct entry_s {
	char *k;
	char *v;
	struct entry_s *next;
} entry_t;

typedef struct hashtable_s {
	int sz;
	struct entry_s **t;	
} ht_t;

ht_t *ht_create(size_t sz);
int hash_function(int sz, char *k);
int hash_function_rep(int rep, int sz, char *k);
int ht_hash(ht_t *h, char *k);
void ht_set(ht_t *h, char *k, char *v);
char *ht_get(ht_t *h, char *k);
void ht_del(ht_t *h, char *k);

#endif /* __HT_H */

