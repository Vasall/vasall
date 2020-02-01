#include "hashtable.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned int hash(const char *key, int size)
{
	unsigned long int value = 0;
	unsigned int i = 0;
	unsigned int key_len = strlen(key);

	/* Do several rounds of multiplication */
	for (; i < key_len; ++i) {
		value = value * 37 + key[i];
	}

	/* Make sure value is 0 <= value < size */
	value = value % size;

	return(value);
}

struct ht_entry *htPair(const char *key, const uint8_t *buf, int size)
{
	struct ht_entry *entry;

	entry = malloc(sizeof(struct ht_entry));
	if(entry == NULL) return (NULL);

	entry->key = NULL;
	entry->buf = NULL;
	entry->next = NULL;

	entry->key = malloc(strlen(key) + 1);
	if(entry->key == NULL) goto failed;
	strcpy(entry->key, key);

	entry->buf = malloc(size);
	if(entry->buf == NULL) goto failed;
	memcpy(entry->buf, buf, size);

	entry->size = size;

	return(entry);

failed:
	free(entry->key);
	free(entry->buf);
	free(entry);

	return(NULL);
}

/*
 * Create a new hash-table. Note that size corresponds to
 * the number of slots/buckets.
 *
 * @size: Number of slots
 *
 * Returns: Either a pointer to the table or NULL
 * 	if an error occurred
 */
struct ht_t *htCreate(int size)
{
	struct ht_t *table;
	int i = 0;

	table = malloc(sizeof(struct ht_t));
	if(table == NULL) return(NULL);

	table->entries = malloc(sizeof(struct ht_entry *) * size);
	if(table->entries == NULL) goto failed;

	for (; i < size; ++i) {
		table->entries[i] = NULL;
	}

	table->size = size;

	return(table);

failed:
	free(table->entries);
	free(table);

	return(NULL);
}

/* 
 * Destroy a hash-table and free allocated memory.
 *
 * @table: Pointer to the table to destroy
 */
void htDestroy(struct ht_t *table)
{
	if(table) {}
}

/*
 * Add a new entry to a hash-table.
 *
 * @table: Pointer to the hash-table
 * @key: The key to attach the data to
 * @buf: The buffer containing the data
 * @size: The size of the buffer in bytes 
 *
 * Returns: Either 0 on success or -1
 * 	if an error occurred
 */
int htSet(struct ht_t *table, const char *key, const uint8_t *buf, int size)
{
	struct ht_entry *entry, *prev;
	unsigned int slot = hash(key, table->size);

	/* Try to look up an entry set */
	entry = table->entries[slot];

	/* No entry means slot empty, insert immediately */
	if (entry == NULL) {
		table->entries[slot] = htPair(key, buf, size);
		if(table->entries[slot] == NULL) return(-1);

		return(0);
	}

	while (entry != NULL) {
		if (strcmp(entry->key, key) == 0) {
			free(entry->buf);
			
			entry->buf = malloc(size);
			if(entry->buf == NULL) return(-1);

			memcpy(entry->buf, buf, size);
			entry->size = size;
			return(0);
		}

		prev = entry;
		entry = prev->next;
	}

	prev->next = htPair(key, buf, size);
	if(prev->next == NULL) return(-1);

	return(0);
}

int htGet(struct ht_t *table, const char *key, uint8_t **ptr, int *size)
{
	struct ht_entry *entry;
	unsigned int slot = hash(key, table->size);

	/* Try to find a valid slot */
	entry = table->entries[slot];
	if (entry == NULL) return(-1);

	while(entry != NULL) {
		if (strcmp(entry->key, key) == 0) {
			*ptr = entry->buf;
			
			if(size != NULL) {
				*size = entry->size;
			}
			return(0);
		}

		entry = entry->next;
	}

	return(-1);
}

void htDel(struct ht_t *table, const char *key)
{
	struct ht_entry *entry, *prev;
	int idx = 0;
	unsigned int bucket = hash(key, table->size);

	entry = table->entries[bucket];
	if (entry == NULL) return;

	while (entry != NULL) {
		if (strcmp(entry->key, key) == 0) {
			if (entry->next == NULL && idx == 0) {
				table->entries[bucket] = NULL;
			}

			if (entry->next != NULL && idx == 0) {
				table->entries[bucket] = entry->next;
			}

			if (entry->next == NULL && idx != 0) {
				prev->next = NULL;
			}

			if (entry->next != NULL && idx != 0) {
				prev->next = entry->next;
			}

			free(entry->key);
			free(entry->buf);
			free(entry);

			return;
		}

		prev = entry;
		entry = prev->next;

		++idx;
	}
}

void htDump(struct ht_t *table)
{
	int i = 0;
	struct ht_entry *entry = NULL;

	for(; i < table->size; i++) {
		printf("[%2x] ", i);

		entry = table->entries[i];

		while(entry != NULL) {
			printf("%s ", entry->key);

			entry = entry->next;
		}	

		printf("\n");
	}
}
