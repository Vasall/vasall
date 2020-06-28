#ifndef _NETWORK_H
#define _NETWORK_H

#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "lcp/inc/lcp.h"

#define PEER_NUM       32
#define PEER_CON_NUM   5

#define PEER_M_NONE        0x00
#define PEER_M_SET         0x01
#define PEER_M_AWAIT       0x02
#define PEER_M_PENDING     0x04
#define PEER_M_CONNECTED   0x08

struct peer_table {
	int num;
	int con_num;
	int pen_num;

	unsigned short         mask[PEER_NUM];
	uint32_t               id[PEER_NUM];
	struct sockaddr_in6    addr[PEER_NUM];
	unsigned short         port[PEER_NUM];
	unsigned char          flag[PEER_NUM];
	struct lcp_con         *con[PEER_NUM];
	unsigned short         obj[PEER_NUM]; 

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

	struct net_evt_ele *evt;

	char status;
	net_cfnc on_success;
	net_cfnc on_failed;
	time_t tout;
		
	uint32_t id;
	uint8_t key[16];
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
 * 
 */
extern int net_update(void);


/* 
 * Handle incoming packets and respond accordingly.
 *
 * @evt: Pointer to the LCP-event
 *
 * Returns: 0 by default
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
extern int net_add_peers(char *buf, int num);


/*
 * Search for a peer in the peer-table by searching to an IPv6-address and port.
 *
 * @addr: The IPv6-address to search for
 * @port: The port-number in the Big-Endian-format to search for
 *
 * Returns: Either the slot in the peer-table or -1 if an error occurred
 */
extern int net_peer_sel_addr(struct in6_addr *addr, unsigned short *port);


/*
 * Search for a peer in the peer-table by searching for a peer-ID.
 *
 * @id: Pointer to a buffer containing the peer-id to search for
 *
 * Returns: Either the slot in the peer-table or -1 if an error occurred
 */
extern int net_peer_sel_id(uint32_t *id);

/*
 * Try to connect to peers from the peer-table and establish reliable
 * connections. 
 *
 * Returns: 0 on success or -1 if an error occurred
 */
extern int net_con_peers(void);

#endif
