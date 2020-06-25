#ifndef _HEADER_H
#define _HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* The request header */
struct req_hdr {
	/* 
	 * The op-code, indicating the type of request or response.
	 */
	uint8_t op;


	uint32_t 
		/* 
		 * Reserved for future use.
		 */
		mod: 12,

		/*
		 * Length of the buffer. Set 0 if no buffer is attached.
		 */
		len: 12;

	/* 
	 * The Peer-ID, indicating the peers the packet should be relayed to.
	 */
	uint32_t dst_id;


	/* 
	 * The Peer-ID, indicating the peer the packet originated from.
	 * 0x00 if unknown, 0x01 for server, 0x02 for service-bot
	 */
	uint32_t src_id;

	/*
	 * Optional peer-key which is practiacally the hashed valued of the
	 * peer-key-buffer combined with the mod-attribute(time % 4086) using 
	 * djb2 by Dan Bernstein. This attribute should only be used if mod 
	 * is bigger than 0.
	 */
	uint32_t src_key;
} __attribute__((__packed__));

/* The size of the header in bytes without the peer-key */
#define REQ_HDR_SIZE     12

/* The size of the header in bytes with the peer-key */
#define REQ_HDR_SIZEW    16


/* Define the op-codes */
#define REQ_OP_SUCCESS      0x01  /* Return success                           */
#define REQ_OP_FAILED       0x02  /* Return failed                            */
#define REQ_OP_INSERT       0x04  /* Insert a new peer into the peer-table    */
#define REQ_OP_REMOVE       0x05  /* Remove peer from the peer-table          */
#define REQ_OP_KEEPALIVE    0x06  /* Prolong peer-timeout                     */
#define REQ_OP_VALIDATE     0x07  /* Validate a peer-id and -key              */
#define REQ_OP_LIST         0x08  /* Request a list of peers                  */
#define REQ_OP_CONVEY       0x09  /* Try to contact peer via middle-man       */
#define REQ_OP_REGISTER     0x0a  /* Register new user                        */
#define REQ_OP_DELETE       0x0b  /* Delete user from database                */


/*
 * Write a header to the given buffer, which has to be allocated already to fit
 * the header. If mod is greater than 0 and a key-buffer is specified, then this
 * function will hash the keybuffer with the modulator and create a 4-bytes
 * hash-value which will then be attached behind the header. If mod is 0, then
 * no key will be written. Note that both the length and the modulator are
 * 12-bits in size and the given values must ot be bigger than 4096!
 *
 * @out: Pointer to write the header to
 * @op: The op-code for this packet
 * @len: The length of the payload (has to be smaller than 4096)
 * @src_id: The peer-id from where the packet originated (0 if unknown)
 * @dst_id: The peer-id to where the packet should be relayed to
 * @mod: The modulator to make the key more secure. (0 if key is unknown)
 * @key: The key-buffer, required mod to be greater than 0
 *
 * Returns: The amount of bytes written to the pointer or -1 if an error
 * 	occurred
 */
extern int hdr_set(char *out, uint8_t op, uint16_t len, uint32_t dst_id, 
		uint32_t src_id, uint16_t mod, uint8_t *key);

/*
 * Extract the values from a header and parse them into the given variables.
 * This function will not check the integrity, but will expect the header to
 * start directly from the given pointer-address. 
 *
 * @in: The buffer to read the header from
 * @op: A pointer to write the op-code to
 * @len: A pointer to write the length of the payload to
 * @src_id: A pointer to write the source-id to
 * @dst_id: A pointer to write the destination-id to
 * @mod: A pointer to write the modulator-value to
 * @key: A pointer to write the hashed key-value to
 *
 * Returns: The number of bytes read from the buffer or -1 if an error occurred
 */
extern int hdr_get(char *in, uint8_t *op, uint16_t *len, uint32_t *dst_id,
		uint32_t *src_id, uint16_t *mod, uint32_t *key);


#ifdef DEF_HEADER

static uint32_t _hash(uint8_t *buf, int l)
{
	unsigned long hash = 5381;
	int i;

	for(i = 0; i < l; i++)
		hash = ((hash << 5) + hash) + buf[i];

	return hash % 0xffffffff;
}

extern int hdr_set(char *out, uint8_t op, uint16_t len, uint32_t dst_id, 
		uint32_t src_id, uint16_t mod, uint8_t *key)
{
	uint8_t key_buf[18];
	uint32_t key_hash;
	struct req_hdr hdr;
	int written = REQ_HDR_SIZE;

	memset(&hdr, 0, 12);
	hdr.op = op;
	hdr.len = len;
	hdr.dst_id = dst_id;
	hdr.src_id = src_id;

	if(mod > 0) {	
		uint32_t tmp;
		memset(key_buf, 0, 16);
		memcpy(key_buf, key, 16);
		memcpy(key_buf + 16, &mod, 2);
		tmp = _hash(key_buf, 18);

		hdr.mod = mod;
		hdr.src_key = tmp;

		written = REQ_HDR_SIZEW;
	}

	memcpy(out, &hdr, written);
	return written;
}

extern int hdr_get(char *in, uint8_t *op, uint16_t *len, uint32_t *dst_id,
		uint32_t *src_id, uint16_t *mod, uint32_t *key)
{
	struct req_hdr hdr;
	int read = REQ_HDR_SIZE;

	memcpy(&hdr, in, REQ_HDR_SIZE);

	if(op) *op = hdr.op;
	if(len) *len = hdr.len;
	if(dst_id) *dst_id = hdr.dst_id;
	if(src_id) *src_id = hdr.src_id;
	if(mod) *mod = hdr.mod;
	if(hdr.mod > 0 && key != NULL) {
		memcpy(key, in + REQ_HDR_SIZE, 4);
		read = REQ_HDR_SIZEW;
	}

	return read;
}

#endif

#endif
