#ifndef _HEADER_H
#define _HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* The request header */
struct req_hdr {
	/* 
	 * The op-code, indicating the type of request or response.
	 */
	uint8_t op;


	/*
	 * The packet-flags indicating certain things, ie if the packet includes
	 * a key.
	 */
	uint8_t flg;

	/*
	 * Reserved bytes for future use.
	 */
	uint16_t res;


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
	 * The time-modulator used for ensuring the safety of the key.
	 */
	uint32_t ti_mod;

	/*
	 * Optional peer-key which is practiacally the hashed valued of the
	 * peer-key-buffer combined with the mod-attribute(time % 4086) using 
	 * djb2 by Dan Bernstein. This attribute should only be used if mod 
	 * is bigger than 0 and the according bit is set in flg.
	 */
	uint32_t src_key;
} __attribute__((__packed__));


/* The size of the header with or without the key attached */
#define HDR_SIZE            12
#define HDR_SIZEW           (HDR_SIZE+8)

/* flags */
#define HDR_F_KEY           0x01

/* 
 * op-codes
 */

#define HDR_OP_OK           0x01  /* Return success                           */
#define HDR_OP_ERR          0x02  /* Return error or failed                   */

#define HDR_OP_INS          0x06  /* Insert a new peer into the peer-table    */
#define HDR_OP_RMV          0x07  /* Remove peer from the peer-table          */
#define HDR_OP_VAL          0x08  /* Validate a peer-id and -key              */
#define HDR_OP_LST          0x09  /* Request a list of peers                  */
#define HDR_OP_CVY          0x0a  /* Try to contact peer via middle-man       */
#define HDR_OP_REG          0x0b  /* Register new user                        */
#define HDR_OP_DEL          0x0c  /* Delete user from database                */

#define HDR_OP_EXC          0x11  /* Exchange object list between two peers   */
#define HDR_OP_GET          0x12  /* Request data about certain objects       */
#define HDR_OP_SBM          0x13  /* Submit a list of objects to a peer       */
#define HDR_OP_UPD          0x14  /* Send a packet containing object-updates  */
#define HDR_OP_CMP          0x15  /*  */
#define HDR_OP_SYN          0x16  /*  */

/*
 * Write a header to the given buffer, which has to be allocated already to fit
 * the header. If mod is greater than 0 and a key-buffer is specified, then this
 * function will hash the keybuffer with a modulator and create a 4-bytes
 * hash-value which will then be attached behind the header. If mod is 0, then
 * no key will be written.
 *
 * @out: Pointer to write the header to
 * @op: The op-code for this packet
 * @src_id: The peer-id from where the packet originated (0 if unknown)
 * @dst_id: The peer-id to where the packet should be relayed to
 * @key: The key-buffer, required mod to be greater than 0
 *
 * Returns: The amount of bytes written to the pointer or -1 if an error
 * 	occurred
 */
extern int hdr_set(char *out, uint8_t op, uint32_t dst_id, 
		uint32_t src_id, uint8_t *key);


/*
 * Extract the values from a header and parse them into the given variables.
 * This function will not check the integrity, but will expect the header to
 * start directly from the given pointer-address. 
 *
 * @in: The buffer to read the header from
 * @op: A pointer to write the op-code to
 * @src_id: A pointer to write the source-id to
 * @dst_id: A pointer to write the destination-id to
 * @mod: A pointer to write the modulator-value to
 * @key: A pointer to write the hashed key-value to
 *
 * Returns: The number of bytes read from the buffer or -1 if an error occurred
 */
extern int hdr_get(char *in, uint8_t *op, uint32_t *dst_id,
		uint32_t *src_id, uint32_t *mod, uint32_t *key);


/*
 * Either extract a header from a buffer or copy one header-struct into another.
 *
 * @in: The source
 * @out: The destination
 *
 * Returns: The number of bytes written or -1 if an error occurred
 */
extern int hdr_cpy(void *in, void *out);

#ifdef DEF_HEADER

static uint32_t _hash(uint8_t *buf, int l)
{
	unsigned long hash = 5381;
	int i;

	for(i = 0; i < l; i++)
		hash = ((hash << 5) + hash) + buf[i];

	return hash % 0xffffffff;
}


extern int hdr_set(char *out, uint8_t op, uint32_t dst_id, 
		uint32_t src_id, uint8_t *key)
{
	uint8_t key_buf[20];
	struct req_hdr hdr;
	int written = HDR_SIZE;

	memset(&hdr, 0, HDR_SIZE);
	hdr.op = op;
	hdr.flg = 0;
	hdr.dst_id = dst_id;
	hdr.src_id = src_id;

	if(key != NULL) {	
		uint32_t ti_mod = time(NULL) % 0xffffffff; 
		uint32_t key_hash;
		memset(key_buf, 0, 16);
		memcpy(key_buf, key, 16);
		memcpy(key_buf + 16, &ti_mod, 4);
		key_hash = _hash(key_buf, 20);

		hdr.flg = hdr.flg | HDR_F_KEY;
		hdr.ti_mod = ti_mod;
		hdr.src_key = key_hash;

		written = HDR_SIZEW;
	}

	memcpy(out, &hdr, written);
	return written;
}


extern int hdr_get(char *in, uint8_t *op, uint32_t *dst_id,
		uint32_t *src_id, uint32_t *mod, uint32_t *key)
{
	struct req_hdr *hdr = (struct req_hdr *)in;
	int read = HDR_SIZE;

	if(op) *op = hdr->op;
	if(dst_id) *dst_id = hdr->dst_id;
	if(src_id) *src_id = hdr->src_id;

	/* If request contains key */
	if((hdr->flg & HDR_F_KEY) == HDR_F_KEY) {
		if(hdr->ti_mod > 0) {
			if(mod)
				*mod = hdr->ti_mod;

			if(key != NULL)
				memcpy(key, in + HDR_SIZE, 4);
		
			read = HDR_SIZEW;
		}
	}

	return read;
}


extern int hdr_cpy(void *in, void *out)
{
	uint8_t flg = *(uint8_t *)((char *)in + 1);
	int len = (flg & HDR_F_KEY) ? HDR_SIZEW : HDR_SIZE;

	memcpy(out, in, len);
	return len;
}

#endif

#endif
