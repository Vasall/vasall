#ifndef _HASHTABLE_H_
#define _HASHTABLE_H_

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

/* Create a new hash-table-entry with this key and data-buffer */
struct ht_entry *htPair(const char *key, const uint8_t *buf, int size);

/* Create a new hash-table */
struct ht_t *htCreate(int size);

/* Destroy a hash-table and free allocated memory */
void htDestroy(struct ht_t *table);

/* Insert data into the table with a given key */
int htSet(struct ht_t *table, const char *key, const uint8_t *buf, int size);

/* Retrieve an entry from the table with a key */
int htGet(struct ht_t *table, const char *key, uint8_t **ptr, int *size);

/* Remove an entry from the table */
void htDel(struct ht_t *table, const char *key);

void htDump(struct ht_t *table);

#endif
