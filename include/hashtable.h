#ifndef _HASHTABLE_H
#define _HASHTABLE_H

#include "defines.h"

#include <stdint.h>

struct ht_entry {
	char *key;
	uint8_t *buf;
	int size;
	struct ht_entry *next;
};

struct ht_t {
	struct ht_entry **entries;

	int size;
};

extern struct ht_t *ht_init(int size);
extern void ht_close(struct ht_t *tbl);

extern int ht_set(struct ht_t *tbl, char *key, uint8_t *buf, int size);
extern void ht_del(struct ht_t *tbl, char *key);

extern int ht_get(struct ht_t *tbl, char *key, uint8_t **ptr, int *size);
extern void ht_print(struct ht_t *tbl);

#endif
