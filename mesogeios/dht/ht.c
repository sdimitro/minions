/* For strdup - feature_test_macros(7) */
#define _XOPEN_SOURCE 500

#include "ht.h"

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>

ht_t *ht_create(size_t sz) {
	ht_t *ht = NULL;

	if (sz < 1) return NULL;
	if (!(ht = malloc(sizeof(ht_t)))) return NULL;
	if (!(ht->t = calloc(sz, sizeof(entry_t *)))) {
		free(ht);
		return NULL;
	}

	ht->sz = sz;
	return ht;	
}

static int hash_value(char *k) {
	unsigned long int hashval = 0;

	unsigned int i = 0;
	while (hashval < ULONG_MAX && i < strlen(k)) {
			hashval = hashval << 8;
			hashval += k[i];
			i++;
		}

	return hashval;
}

int hash_function(int sz, char *k) {
	return hash_value(k) % sz;
}

int hash_function_rep(int rep, int sz, char *k) {
	return (hash_value(k) + rep) % sz;
}

int ht_hash(ht_t *h, char *k) {
	return hash_value(k) % h->sz;
}

static entry_t *ht_newpair(char *k, char *v) {
	entry_t *newpair;

	if (!(newpair = calloc(1, sizeof(entry_t))))
			return NULL;

	if (!(newpair->k = strdup(k))) {
		free(newpair);
		return NULL;
	}

	if (!(newpair->v = strdup(v))) {
		free(newpair);
		return NULL;
	}
	return newpair;
}

void ht_set(ht_t *h, char *k, char *v) {
	int bin = 0;
	entry_t *newpair = NULL;
	entry_t *next = NULL;
	entry_t *last = NULL;

	bin = ht_hash(h, k);
	next = h->t[bin];

	while (next && next->k && strcmp(k, next->k) > 0) {
		last = next;
		next = next->next;
	}

	if (next && next->k && !strcmp(k, next->k)) {
		free(next->v);
		next->v = strdup(v);
	} else {
		newpair = ht_newpair(k, v);

		if (next == h->t[bin]) {
			newpair->next = next;
			h->t[bin] = newpair;
		} else if (!next) {
			last->next = newpair;
		} else  {
			newpair->next = next;
			last->next = newpair;
		}
	}
}

char *ht_get(ht_t *h, char *k) {
	int bin = 0;
	entry_t *pair;

	bin = ht_hash(h, k);

	pair = h->t[bin];
	while (pair && pair->k && strcmp(k, pair->k) > 0 )
		pair = pair->next;

	if (!pair || !(pair->k) || strcmp(k, pair->k))
		return NULL;
	
	return pair->v;
}

void ht_del(ht_t *h, char *k) {
	ht_set(h, k, "\0");
}

