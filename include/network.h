#ifndef _NETWORK_H
#define _NETWORK_H

#include "upnp.h"
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>

/* An entry in the send-que, to verify the packet reached it's destination */
struct sock_pck_que {
	struct lcp_pck_que *next;

	char *buf;
	int len;

	char count;
	time_t tout;
};

/* 10 client-sockets + 1 server-socket */
#define SOCK_NUM       11
#define SOCK_MIN_PORT  25253

#define SOCK_M_NONE    0x00
#define SOCK_M_INIT    0x01
#define SOCK_M_USE     0x02
#define SOCK_M_PPR     0x04
#define SOCK_M_UPNP    0x08

struct socket_table {	
	uint8_t mask[SOCK_NUM];
	int fd[SOCK_NUM];

	unsigned short int_port[SOCK_NUM];
	unsigned short ext_port[SOCK_NUM];

	struct sockaddr_in6 dst[SOCK_NUM];

	time_t tout[SOCK_NUM];

	uint8_t status[SOCK_NUM];
	struct sock_pck_que *que[SOCK_NUM];
};


#define NET_F_PPR      0x01
#define NET_F_UPNP     0x02
#define NET_F_PMP      0x04
#define NET_F_PCP      0x08

/*
 * Define the IPv6-addresses and ports of the default servers.
 */
#define MAIN_IP        "0:0:0:0:0:ffff:4e2e:bbb1\0"
#define MAIN_PORT      4242

#define DISCO_IP       "0:0:0:0:0:ffff:4e2f:27b2\0"
#define DISCO_PORT     4243

#define PROXY_IP       "0:0:0:0:0:ffff:4e2f:27b2"
#define PROXY_PORT     4244 

struct network_wrapper {
	struct socket_table sock;

	struct in6_addr int_addr;
	struct in6_addr ext_addr;
	uint8_t flg;

	struct sockaddr_in6 main_addr;
	struct sockaddr_in6 disco_addr;
	struct sockaddr_in6 proxy_addr;

	struct upnp_handle upnp;
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


#define NET_USEANY -1
#define NET_USENEW -2

/*
 * Send a packet to the given destination. This function will use a given slot,
 * use a new slot or create a new socket to send the message, depending on the
 * value passed in slot. Sending packets using this function is not safe, as
 * this function will not check if the packet reached it's destination. To have
 * a safe option, use lcp_send() instead.
 *
 * @slot: LCP_USENEW for a new socket, LCP_USEANY for an unused slot and an 
 * 	  index for a slot
 * @dst: The destination to send the packet to
 * @buf: The buffer to send
 * @len: The length of the buffer in bytes
 *
 * Returns: 0 on success or -1 if an error occurred
 */
extern int net_sendto(short slot, struct sockaddr_in6 *dst, char *buf, int len);


/*
 * Send a packet to the given destination and verify that it reached it's
 * destination. If the transmission failed, this function will retry sending
 * the packet to the destination two more times. If that failed, an error-evt
 * will be pushed into the event-queue.
 *
 * @slot: The socket-slot to use (has to be connected already)
 * @buf: The buffer to send to the other maschine
 * @len: The length of the buffer
 *
 * Returns: 0 on success or -1 if an error occurred
 */
extern int net_send(short slot, char *buf, int len);


/*
 * Connect to a different maschine and establish a connection.
 *
 * @dst: The address of the other maschine to connect to
 * @flg: Options on how to establish the connection
 *
 * Returns: Either a slot in the socket-table or -1 if an error occurred
 */
extern short net_connect(struct sockaddr_in6 *dst, uint8_t flg);


/*
 * Close a connection and reset the socket.
 *
 * @slot: The slot in the socket-table
 */
extern void net_disconnect(short slot);


/*
 *  
 */
extern void net_process(void);


/*
 * 
 */
extern void net_print_sock(void);


/*
 * Convert a binary IPv4-address to a binary IPv6-address.
 *
 * @src: The IPv4-address to convert
 * @dst: The IPv6-address to convert
 *
 * Returns: 0 on success or -1 if an error occurred
 */
extern int net_btob_4to6(struct in_addr *src, struct in6_addr *dst);


/*
 * Convert an IPv6-address to a string.
 * IMPORTANT: This function is not thread-safe!
 *
 * @addr: Pointer to the address
 *
 * Returns: A string containing the address in text form
 */
extern char *net_str_addr6(struct in6_addr *addr);



#endif
