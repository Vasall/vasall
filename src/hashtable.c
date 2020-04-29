#include "hashtable.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


V_API unsigned int hash(const char *key, int size)
{
	unsigned long int val = 0;
	unsigned int i = 0;
	unsigned int len = strlen(key);

	for(; i < len; i++)
		val = val * 37 + key[i];

	return val % size;
}

V_API struct ht_entry *ht_pair(const char *key, const uint8_t *buf, int size)
{
	struct ht_entry *entry;

	if(!(entry = malloc(sizeof(struct ht_entry))))
		return NULL;

	entry->key = NULL;
	entry->buf = NULL;
	entry->next = NULL;

	if(!(entry->key = malloc(strlen(key) + 1)))
		goto err_free_entry;

	strcpy(entry->key, key);

	if(!(entry->buf = malloc(size)))
		goto err_free_entry;

	memcpy(entry->buf, buf, size);
	entry->size = size;
	return entry;

err_free_entry:
	free(entry->key);
	free(entry->buf);
	free(entry);
	return NULL;
}

V_API struct ht_t *ht_init(int size)
{
	struct ht_t *tbl;
	int i = 0;
	int tmp;

	if(!(tbl = malloc(sizeof(struct ht_t))))
		return NULL;

	tmp = sizeof(struct ht_entry *);
	if(!(tbl->entries = malloc(tmp * size)))
		goto err_free_tbl;

	for(; i < size; ++i)
		tbl->entries[i] = NULL;

	tbl->size = size;
	return tbl;

err_free_tbl:
	free(tbl->entries);
	free(tbl);
	return NULL;
}

V_API void ht_close(struct ht_t *tbl)
{
	int i;
	struct ht_entry *ptr, *next;

	for(i = 0; i < tbl->size; i++) {
		ptr = tbl->entries[i];

		while(ptr) {
			next = ptr->next;

			free(ptr->key);		
			free(ptr->buf);
			free(ptr);

			ptr = next;
		}
	}

	free(tbl->entries);
	free(tbl);
}

V_API int ht_set(struct ht_t *tbl, const char *key, const uint8_t *buf, 
		int size)
{
	struct ht_entry *ptr, *prev;
	unsigned int slot = hash(key, tbl->size);

	/* Try to look up an entry set */
	ptr = tbl->entries[slot];

	/* No entry means slot empty, insert immediately */
	if(!ptr) {
		if(!(tbl->entries[slot] = ht_pair(key, buf, size)))
			return -1;

		return 0;
	}

	while(ptr) {
		if(!strcmp(ptr->key, key)) {
			free(ptr->buf);
			
			if(!(ptr->buf = malloc(size)))
				return -1;

			memcpy(ptr->buf, buf, size);
			ptr->size = size;
			return 0;
		}

		prev = ptr;
		ptr = ptr->next;
	}

	if(!(prev->next = ht_pair(key, buf, size)))
		return -1;

	return 0;
}

V_API int ht_get(struct ht_t *tbl, const char *key, uint8_t **ptr, int *size)
{
	struct ht_entry *entry;
	unsigned int slot = hash(key, tbl->size);

	/* Try to find a valid slot */
	if(!(entry = tbl->entries[slot]))
		return -1;

	while(entry != NULL) {
		if (strcmp(entry->key, key) == 0) {
			*ptr = entry->buf;
			
			if(size)
				*size = entry->size;
			
			return 0;
		}

		entry = entry->next;
	}

	return -1;
}

V_API void ht_del(struct ht_t *tbl, const char *key)
{
	struct ht_entry *ptr;
	struct ht_entry *prev;
	int idx = 0;
	unsigned int bucket = hash(key, tbl->size);

	if(!(ptr = tbl->entries[bucket]))
		return;

	while(ptr) {
		if (strcmp(ptr->key, key) == 0) {
			if (!ptr->next && !idx)
				tbl->entries[bucket] = NULL;

			else if(ptr->next && !idx)
				tbl->entries[bucket] = ptr->next;

			else if(!ptr->next && idx)
				prev->next = NULL;

			else if(ptr->next && idx)
				prev->next = ptr->next;

			free(ptr->key);
			free(ptr->buf);
			free(ptr);
			return;
		}

		prev = ptr;
		ptr = ptr->next;
		idx++;
	}
}

V_API void ht_print(struct ht_t *tbl)
{
	int i = 0;
	struct ht_entry *ptr = NULL;

	for(; i < tbl->size; i++) {
		printf("[%2x] ", i);

		ptr = tbl->entries[i];

		while(ptr) {
			printf("%s ", ptr->key);
			ptr = ptr->next;
		}	

		printf("\n");
	}
}
