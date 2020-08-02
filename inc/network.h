#ifndef _NETWORK_H
#define _NETWORK_H

#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "lcp/inc/lcp.h"

#define PEER_SLOTS         18
#define PEER_CON_NUM       6

#define PEER_M_NONE        0x00
#define PEER_M_SET         0x01
#define PEER_M_CON         0x02

#define PEER_S_SET         0x01
#define PEER_S_AWA         0x02
#define PEER_S_PEN         0x04
#define PEER_S_CON         0x08
#define PEER_S_EXC         0x10
#define PEER_S_SYN         0x20
#define PEER_S_7           0x40
#define PEER_S_8           0x80

struct peer_table {
	int num;
	int con_num;
	int pen_num;

	unsigned char          mask[PEER_SLOTS];

	unsigned char          status[PEER_SLOTS];
	time_t                 tout[PEER_SLOTS];

	uint32_t               id[PEER_SLOTS];
	uint32_t               mdl[PEER_SLOTS];
	struct sockaddr_in6    addr[PEER_SLOTS];
	unsigned short         port[PEER_SLOTS];
	unsigned char          flag[PEER_SLOTS];
	struct lcp_con         *con[PEER_SLOTS];
	unsigned short         obj[PEER_SLOTS][1]; 
};

struct cache_entry;
struct cache_entry {
	struct cache_entry *next;
	struct cache_entry *prev;
	uint32_t id;
	uint32_t src;

	char status;
	time_t tout;
};

/*
 * Define the IPv6-addresses and ports of the default servers.
 */
#define USE_LOCAL 1

#if !USE_LOCAL
#define MAIN_IP        "0:0:0:0:0:ffff:4e2e:bbb1"
#else
#define MAIN_IP        "::1"
#endif
#define MAIN_PORT      4242

#if !USE_LOCAL
#define DISCO_IP       "0:0:0:0:0:ffff:4e2f:27b2"
#else
#define DISCO_IP       "::1"
#endif
#define DISCO_PORT     4243

#if !USE_LOCAL
#define PROXY_IP       "0:0:0:0:0:ffff:4e2f:27b2"
#else
#define PROXY_IP       "::1"
#endif
#define PROXY_PORT     4244 

/* Callback-function for network-events */
typedef void (*net_cfnc)(char *buf, int len);

struct network_wrapper {
	struct lcp_ctx *ctx;

	struct sockaddr_in6 main_addr;

	struct peer_table peers;

	short con_num;
	short con[PEER_CON_NUM];

	struct net_evt_ele *evt;

	char status;
	net_cfnc on_success;
	net_cfnc on_failed;
	time_t tout;
		
	uint32_t id;
	uint8_t key[16];

	struct cache_entry *cache;

	/* Time difference to the universal server-timer */
	uint32_t time_del;


	/* Varaibles used for debugging and prototyping */
	int count;
};


/* Define the global network-wrapper instance */
extern struct network_wrapper network;


/*
 * Gather all information about the NAT, the internal and external network. 
 * Then initialize the socket-table and setup the sockets. Also use uPnP to 
 * forward ports on the NAT if possible.
 *
 * Returns: 0 on success or -1 if an error occurred
 */
extern int net_init(void);


/*
 * Close the socket table and close all open sockets. If uPnP is enabled also
 * remove entries from the NAT.
 */
extern void net_close(void);


/*
 * Register incoming packets and process requests.
 *
 * Returns: 0 on success or -1 if an error occurred
 */
extern int net_update(void);


/* 
 * Handle incoming packets and respond accordingly.
 *
 * @evt: Pointer to the LCP-event
 *
 * Returns: This function will always return 0
 */
extern int peer_handle(struct lcp_evt *evt);


/*
 * Create a new key and insert yourself as a new peer into the peer-list.
 * Note that this function will block progression, until the server respondes or
 * if the connection timed out. Once a reponse has come, the values will be
 * filled in if successfull, and the function will return the according value.
 *
 * @uname: The buffer containing the null-terminated username
 * @pswd: The buffer containing the null-terminated unencrypted password
 * @on_success: A callback-function in case the request is successfull
 * @on_failed: A callback-function in case the request failed
 *
 * Returns: 1 on success, 0 if data are invalid and -1 if an error occurred
 */
extern int net_insert(char *uname, char *pswd, net_cfnc on_success, 
		net_cfnc on_failed);


/*
 * Request a list of peers from the server.
 *
 * @on_success: A callback-function in case the request is successfull
 * @on_failed: A callback-function in case the request failed
 *
 * Returns: 0 on success or -1 if an error occurred
 */
extern int net_list(net_cfnc on_success, net_cfnc on_failed);


/*
 * Enter a new peer into the peer-table and await further instructions from the
 * middleman.
 *
 * @id: Pointer to buffer containing the ID of the new peer
 *
 * Returns: The slot-number on success or -1 if an error occurred
 */
extern int net_add_peer(uint32_t *id);


/*
 * Process a received peer-buffer and inser the received buffers into the
 * peer-table. For each peer, the following data is required:
 * - The peer-id (4 bytes)
 * - The peer-IPv6-address (16 bytes)
 * - The peer-port (2 bytes)
 * - The peer-connection-flag (1 byte)
 *
 * @buf: The buffer containing the peer-data
 * @num: The number of peers in the buffer
 *
 * Returns: 0 on success or -1 if an error occurred
 */
extern int net_add_peers(char *buf, short num);


/*
 * Search for a peer in the peer-table by searching to an IPv6-address and port.
 *
 * @addr: The IPv6-address to search for
 * @port: The port-number in the Big-Endian-format to search for
 *
 * Returns: Either the slot in the peer-table or -1 if an error occurred
 */
extern short net_peer_sel_addr(struct in6_addr *addr, unsigned short *port);


/*
 * Search for a peer in the peer-table by searching for a peer-ID.
 *
 * @id: Pointer to a buffer containing the peer-id to search for
 *
 * Returns: Either the slot in the peer-table or -1 if an error occurred
 */
extern short net_peer_sel_id(uint32_t *id);


/*
 * Try to connect to peers from the peer-table and establish reliable
 * connections. 
 *
 * Returns: 0 on success or -1 if an error occurred
 */
extern int net_con_peers(void);


/*
 * Add a peer-index to the list of connected peers.
 *
 * @slot: The slot of the peer in the peer-table
 *
 * Returns: Either the index in the connected-table or -1 if an error occurred
 */
extern int net_con_add(short slot);


/*
 * Remove a peer-index from the list of connected-peers.
 *
 * @slot: The slot of the peer in the peer-table
 */
extern void net_con_remv(short slot);


/* 
 *
 */
extern int net_broadcast(char *buf, int len);

/*
 * Insert a list of object-id into the object-cache and then get the list of
 * objects that still have to be requested from other peers. Note that the
 * returned id-buffer has to be freed after usage to prevent memory-leaks.
 * This function has the purpose to prevent duplicated and enable
 * object-synchronization with multible peers at the same time, therefore it
 * will not yet push objects into the object-table. Note that the first two
 * bytes of the returned buffer contain the number of ids written.
 *  
 * @in: The list of id(each id has to be 4 bytes)
 * @in_num: The number of ids starting from the given pointer-address
 * @src: The id of the peer that sent the list of ids
 * @out: A pointer to attach the object-ids to that still have to be set up
 * @out_num: A pointer to write the number of returned ids to
 *
 * Returns: 0 on success or -1 if an error occurred
 */
extern int net_obj_insert(void *in, short in_num, uint32_t src, char **out,
		short *out_num);


/*
 * Find an object with a given id in the object-cache and return the pointer to
 * that object.
 *
 * @id: The object-id to search for
 *
 * Returns: Either a pointer to the cache-object or NULL if an error occurred
 */
extern struct cache_entry *net_obj_find(uint32_t id);


/*
 * This function will remove the objects from the object-cache and push them
 * into the object-table with the given dataset. Note that to prevent hacking,
 * the source-peer-id has to be equal to the peer-id from where the original
 * object-id came from.
 *
 * @ptr: The address containing the object-data
 * @num: The number of objects in the buffer
 * @src: The id of the peer that sent the object-data
 *
 * Returns: 0 on success or -1 if an error occurred
 */
extern int net_obj_submit(void *ptr, uint32_t ts, short num, uint32_t src);


/*
 * Go through the object-cache and send requests to the different peers
 * containing the list of the required objects.
 *
 * Returns: 0 on success or -1 if an error occurred
 */
extern int net_obj_update(void);

#endif
