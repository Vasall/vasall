#include "network.h"
#include "error.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <ifaddrs.h>


/* Redefine global network-wrapper */
struct network_wrapper network;


/* Initialize the server-structs */
static int init_serv(void)
{
	int size = sizeof(struct sockaddr_in6);

	/* Setup the default server-addresses */
	memset(&network.main_addr, 0, size);
	network.main_addr.sin6_family = AF_INET6;
	network.main_addr.sin6_port = htons(MAIN_PORT);
	if(inet_pton(AF_INET6, MAIN_IP, &network.main_addr.sin6_addr) < 0) {
		ERR_LOG(("Failed to convert main-addr to binary"));
		return -1;
	}

	memset(&network.disco_addr, 0, size);
	network.disco_addr.sin6_family = AF_INET6;
	network.disco_addr.sin6_port = htons(DISCO_PORT);
	if(inet_pton(AF_INET6, DISCO_IP, &network.disco_addr.sin6_addr) < 0) {
		ERR_LOG(("Failed to convert disco-addr to binary"));
		return -1;
	}

	memset(&network.proxy_addr, 0, size);
	network.proxy_addr.sin6_family = AF_INET6;
	network.proxy_addr.sin6_port = htons(PROXY_PORT);
	if(inet_pton(AF_INET6, PROXY_IP, &network.proxy_addr.sin6_addr) < 0) {
		ERR_LOG(("Failed to convert proxy-addr to binary"));
		return -1;
	}

	return 0;
}


/* Get the external address and check if port preservation is enabled */
static int net_discover(void)
{
	int sockfd;
	int port;
	struct sockaddr_in6 cli;
	struct sockaddr *cli_ptr = (struct sockaddr *)&cli;
	struct sockaddr *serv_ptr;
	struct sockaddr_in6 res;
	int size = sizeof(struct sockaddr_in6);
	struct timeval tv;
	int tv_sz = sizeof(struct timeval);

	if((sockfd = socket(PF_INET6, SOCK_DGRAM, 0)) < 0) {
		ERR_LOG(("Failed to create socket"));
		return -1;
	}

	/* Set timeout for receiving data */
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, tv_sz) < 0) {
		ERR_LOG(("Failed to set timeout"));
		goto err_close_sockfd;
	}

	/* Choose a random port */
	port = 27000 + (rand() % 2525);

	/* Bind the socket to the port */
	memset(&cli, 0, size);
	cli.sin6_family = AF_INET6;
	cli.sin6_port = htons(port);
	cli.sin6_addr = in6addr_any;
	if(bind(sockfd, cli_ptr, size) < 0) {
		ERR_LOG(("bind()"));
		goto err_close_sockfd;
	}

	serv_ptr = (struct sockaddr *)&network.disco_addr;

	/* Send a request to the stun-server */
	if(sendto(sockfd, "hi\0", 3, 0, serv_ptr, size) < 0) {
		ERR_LOG(("Failed to send data"));
		goto err_close_sockfd;
	}

	/* Listen for a response from the server */
	if(recvfrom(sockfd, &res, size, 0, NULL, NULL) < 0) {
		ERR_LOG(("Failed to receive data"));
		goto err_close_sockfd;
	}

	/* Copy the external IPv6-address and port */
	network.ext_addr = res.sin6_addr;

	if(ntohs(res.sin6_port) == port) {
		network.flg = network.flg | NET_F_PPR;
	}

	close(sockfd);
	return 0;

err_close_sockfd:
	close(sockfd);
	return -1;
}


/* Get the internal address */
static int net_get_intern(void)
{
	struct ifaddrs *addrs;
	struct ifaddrs *tmp;
	struct sockaddr_in *paddr;
	int ret = -1;

	getifaddrs(&addrs);
	tmp = addrs;

	while(tmp) {
		if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET) {
			paddr = (struct sockaddr_in *)tmp->ifa_addr;
			if(*(int *)&paddr->sin_addr != 0x0100007F) {
				net_btob_4to6(&paddr->sin_addr, &network.int_addr);
				ret = 0;
				break;
			}
		}

		tmp = tmp->ifa_next;
	}

	freeifaddrs(addrs);
	return ret;
}


extern int net_init(void)
{
	int sockfd;
	int port;
	int i;
	struct sockaddr_in6 addr;
	struct sockaddr *addr_ptr = (struct sockaddr *)&addr;
	int addr_sz = sizeof(struct sockaddr_in6);
	struct socket_table *tbl = &network.sock;

	/* Initialize the basic address-structs */
	if(init_serv() < 0) {
		ERR_LOG(("Failed to setup server-addresses"));
		return -1;
	}

	/* Discover the external address and test port preservation */
	if(net_discover() < 0) {
		ERR_LOG(("Failed to contact discovery-server"));
		return -1;
	}

	/* Discover the internal address */
	if(net_get_intern() < 0) {
		ERR_LOG(("Failed to get internal address"));
		return -1;
	}

	/* Setup all sockets */
	for(i = 0; i < SOCK_NUM; i++) {
		port = SOCK_MIN_PORT + i;

		if((sockfd = socket(PF_INET6, SOCK_DGRAM, 0)) < 0) {
			ERR_LOG(("Failed to create socket"));
			goto err_close_socks;
		}

		tbl->mask[i] = SOCK_M_INIT;
		tbl->fd[i] = sockfd;
		tbl->int_port[i] = port;
		tbl->ext_port[i] = port;
		tbl->tout[i] = -1;
		tbl->status[i] = 0;
		tbl->que[i] = NULL;

		memset(&addr, 0, addr_sz);
		addr.sin6_family = AF_INET6;
		addr.sin6_port = htons(port);
		addr.sin6_addr = in6addr_any;

		/* Bind the socket to the  */
		if(bind(sockfd, addr_ptr, addr_sz) < 0) {
			ERR_LOG(("Failed to bind port"));
			goto err_close_socks;
		}

		/* Set socket non-blocking */
		if(fcntl(sockfd, F_SETFL, O_NONBLOCK)  < 0) {
			ERR_LOG(("Failed to set port non-blocking"));
			goto err_close_socks;
		}

		/* Create a new uPnP entry on the NAT if possible */
		if((network.flg & NET_F_UPNP) != 0 ) {
			if(upnp_add(&network.upnp, port, port) != 0) {
				ERR_LOG(("Failed to forward port using uPnP"));
				goto err_close_socks;
			}

			/* Mark upnp as active */
			tbl->mask[i] += SOCK_M_UPNP;
		}
	}

	return 0;

err_close_socks:
	/* Close all opened sockets */
	for(; i >= 0; i--) {
		if(tbl->mask[i] == 0)
			continue;

		if((tbl->mask[i] & SOCK_M_UPNP) != 0)
			upnp_remv(&network.upnp, tbl->ext_port[i]);

		close(tbl->fd[i]);

		/* Reset the mask of the entry */
		tbl->mask[i] = 0;
	}

	return -1;
}


extern void net_close(void)
{
	int i;
	struct socket_table *tbl = &network.sock;

	for(i = 0; i < SOCK_NUM; i++) {
		if(tbl->mask[i] == 0)
			continue;

		if((tbl->mask[i] & SOCK_M_UPNP) != 0)
			upnp_remv(&network.upnp, tbl->ext_port[i]);

		close(tbl->fd[i]);

		/* Reset the mask of the entry */
		tbl->mask[i] = 0;
	}
}


extern void net_update(void)
{
	int i;
	struct socket_table *tbl = &network.sock;

	for(i = 0; i < SOCK_NUM; i++) {
		if(tbl->mask[i] == 0)
			continue;
	}
}


extern void net_print_sock(void)
{
	int i;
	struct socket_table *tbl = &network.sock;

	printf("NUM\tMASK\tFD\tINT_PORT\tEXT_PORT\tTOUT\n");
	for(i = 0; i < SOCK_NUM; i++) {
		printf("%d\t%d\t%d\t%d   \t%d   \t%lu\n", i,
				tbl->mask[i], tbl->fd[i], 
				tbl->int_port[i], tbl->ext_port[i], 
				tbl->tout[i]);
	}
	printf("\n");
}


extern int net_btob_4to6(struct in_addr *src, struct in6_addr *dst)
{
	char *ptr = (char *)dst;
	memset(ptr, 0, sizeof(struct in6_addr));
	memset(ptr + 10, 0xff, 2);
	memcpy(ptr + 12, src, sizeof(struct in_addr));
	return 0;
}


/* NOTE: This function is not thread safe */
extern char *net_str_addr6(struct in6_addr *addr)
{
	static char buf[INET6_ADDRSTRLEN];
	if(inet_ntop(AF_INET6, addr, buf, INET6_ADDRSTRLEN) == NULL)
		ERR_LOG(("Failed to convert address to text"));
	return buf;
}
