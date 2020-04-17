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

/*
 * Hash a key and consider the amount of slots in a table.
 *
 * @key: The null-terminated key to be hashed
 * @size: The number of slots in a table
 *
 * Returns: The hashed-key
 */
unsigned int hash(const char *key, int size);

/*
 * Create a new hashtable-entry and bind both the key and the buffer to it.
 * The returned pointer can then be inserted into the table.
 *
 * @key: The key of this entry
 * @buf: The data-buffer to attach
 * @size: The size of the buffer in bytes
 *
 * Returns: Either a pointer to a new entry or NULL if an error occurred
 */
struct ht_entry *ht_pair(const char *key, const uint8_t *buf, int size);

/*
 * Create a new hash-table. Note that size corresponds to
 * the number of slots/buckets.
 *
 * @size: Number of slots
 *
 * Returns: Either a pointer to the table or NULL
 * 	if an error occurred
 */
struct ht_t *ht_init(int size);

/* 
 * Destroy a hash-table and free allocated memory.
 *
 * @tbl: Pointer to the table to destroy
 */
void ht_close(struct ht_t *tbl);

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
int ht_set(struct ht_t *tbl, const char *key, const uint8_t *buf, int size);

/*
 * Retrieve data from a hashtable. The pointer to the
 * buffer and the size of the buffer in bytes will be
 * written to the parameters.
 *
 * @tbl: Pointer to the table
 * @key: The key of the entry to get
 * @ptr: A pointer to write the buffer-address to
 * @size: A pointer to write the buffer-size to(can be NULL)
 *
 * Returns: Either 0 on success or -1
 * 	if an error occurred
 */
int ht_get(struct ht_t *tbl, const char *key, uint8_t **ptr, int *size);

/*
 * Delete and remove an entry from the hashtable.
 *
 * @tbl: Pointer to the hashtable
 * @key: The key of the entry
 */
void ht_del(struct ht_t *tbl, const char *key);

/*
 * Dump info about a hashtable into
 * the terminal.
 *
 * @tbl: Pointer to the table to dump
 */
void ht_print(struct ht_t *tbl);

#endif
