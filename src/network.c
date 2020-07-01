#include "network.h"
#include "error.h"
#define DEF_HEADER
#include "header.h"

#include "core.h"

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
#include <openssl/sha.h>

/* Redefine global network-wrapper */
struct network_wrapper network;


static int net_peer_init(void)
{
	int i;
	struct peer_table *tbl = &network.peers;

	tbl->num = 0;
	tbl->con_num = 0;
	tbl->pen_num = 0;

	for(i = 0; i < PEER_SLOTS; i++)
		tbl->mask[i] = PEER_M_NONE;

	return 0;
}


extern int net_init(void)
{
	struct sockaddr_in6 disco;
	struct sockaddr_in6 proxy;
	int tmp = sizeof(struct sockaddr_in6);
	struct lcp_evt evt;
	char running = 1;

	/* Set the address and port of the disco-server */
	memset(&network.main_addr, 0, tmp);
	network.main_addr.sin6_family = AF_INET6;
	network.main_addr.sin6_port = htons(MAIN_PORT);
	if(inet_pton(AF_INET6, MAIN_IP, &network.main_addr.sin6_addr) < 0)
		return -1;

	/* Set the address and port of the disco-server */
	memset(&disco, 0, tmp);
	disco.sin6_family = AF_INET6;
	disco.sin6_port = htons(DISCO_PORT);
	if(inet_pton(AF_INET6, DISCO_IP, &disco.sin6_addr) < 0)
		return -1;

	/* Set the address and port of the proxy-server */
	memset(&proxy, 0, tmp);
	proxy.sin6_family = AF_INET6;
	proxy.sin6_port = htons(PROXY_PORT);
	if(inet_pton(AF_INET6, PROXY_IP, &proxy.sin6_addr) < 0)
		return -1;

	/* Initialize the LCP-context */
	if(!(network.ctx = lcp_init(rand() % 3000 + 10000, 0, 0, &disco, &proxy)))
		return -1;

	/* Set initial values */
	network.status = 0;
	network.tout = 0;

	/* Initialize the peer-table */
	if(net_peer_init() < 0)
		goto err_close_ctx;


	printf("Connect to server: %s\n", lcp_str_addr6(&network.main_addr));

	/* Connect to server */
	if(!(lcp_connect(network.ctx, -1, &network.main_addr, LCP_CON_F_DIRECT,
					LCP_F_ENC)))
		goto err_close_ctx;

	while(running) {
		lcp_update(network.ctx);

		while(lcp_pull_evt(network.ctx, &evt)) {	
			if(evt.type == LCP_CONNECTED) {
				running = 0;
				break;
			}
			else if(evt.type == LCP_FAILED || 
					evt.type == LCP_TIMEDOUT) {
				goto err_close_ctx;
			}
			lcp_del_evt(&evt);
		}
	}

	return 0;

err_close_ctx:
	lcp_close(network.ctx);
	return -1;
}


extern void net_close(void)
{
	/* Close LCP-context */
	lcp_close(network.ctx);
;}


extern int net_update(void)
{
	struct lcp_evt evt;
	time_t ti;
	struct peer_table *tbl = &network.peers;

	/* Processing incoming packets and send out requests */
	lcp_update(network.ctx);

	while(lcp_pull_evt(network.ctx, &evt)) {
		if(evt.type == LCP_CONNECTED) {
			short slot;
			struct in6_addr addr;
			unsigned short port;

			memcpy(&addr, &evt.addr.sin6_addr, 16);
			port = evt.addr.sin6_port;

			printf("Atta\n");

			/* Mark peer as connected */
			if((slot = net_peer_sel_addr(&addr, &port)) >= 0) {
				/* Update entry-mask */
				tbl->status[slot] = PEER_S_CONNECTED;

				/* Update num of connected and pending peers */
				tbl->con_num++;
				tbl->pen_num--;

				printf("Connected to %s\n", 
						lcp_str_addr6(&evt.addr));
			}
		}
		else if(evt.type == LCP_RECEIVED) {
			/* Handle packets if they seem valid */
			if(evt.len >= REQ_HDR_SIZE)
				peer_handle(&evt);
		}
		else if(evt.type == LCP_FAILED || evt.type == LCP_TIMEDOUT) {
			printf("Couldn't connect to peer\n");
		}

		lcp_del_evt(&evt);
	}

	time(&ti);

	if(network.status == 0x02) {
		int num = tbl->con_num + tbl->pen_num;
		int i;

		/* Get new peers from the server */
		if(tbl->num == 0 && ti > network.tout) {
			/* Send a request to the server */
			net_list(NULL, NULL);

			/* Update timeout */
			network.tout = ti + 5;
		}
		/* Connect to peers if necessary */
		else if(num < 1 && tbl->num > 0) {
			net_con_peers();
		}


		for(i = 0; i < PEER_SLOTS; i++) {
			if(tbl->mask[i] == PEER_M_NONE)
				continue;

			/* Yet to synchronize with peer */
			if((tbl->status[i] & PEER_S_SYNCED) == 0) {
					
			}
		}
	}

	return 0;
}


extern int peer_handle(struct lcp_evt *evt)
{
	char *ptr;
	int tmp;

	uint8_t op;
	uint16_t len;
	uint32_t dst_id;
	uint32_t src_id;
	uint32_t mod;
	uint32_t key;

	char pck[512];

	struct peer_table *tbl = &network.peers;
	struct lcp_ctx *ctx = network.ctx;

	/* Extract data from header */
	tmp = hdr_get(evt->buf, &op, &dst_id, &src_id, &mod, &key);
	len = evt->len - tmp;

	if(len) {/* Prevent warning for not using len yet */}

	/* Set pointer */
	ptr = evt->buf + tmp;

	if(op == REQ_OP_INSERT) {
		if(network.status == 1) {
			uint32_t id;
			uint32_t mask;
			float pos[3];
			int mdl;
			short slot;
			short peer_num;

			/* If the request got rejected */
			if(ptr[0] != 1) {
				/* Update status */
				network.status = 0;

				/* Run callback-function */
				if(network.on_failed != NULL)
					network.on_failed(NULL, 0);

				return 0;
			}

			/* Copy both the peer-id and -key */
			memcpy(&network.id, ptr + 1, 4);
			memcpy(network.key, ptr + 5, 16);

			/* Insert object into object-table */
			id = network.id;
			memcpy(pos, ptr + 21, 3 * sizeof(float));
			mask = OBJ_M_ENTITY;
			mdl = mdl_get("plr");

			if((slot = obj_set(id, mask, pos, mdl, NULL, 0)) < 0) {
				/* Run callback-function */
				if(network.on_failed != NULL)
					network.on_failed(NULL, 0);

				return 0;
			}

			/* Set the index of the core-object */
			core.obj = slot;

			/* Check if any peers are includes */
			memcpy(&peer_num, ptr + 33, 2);
			if(peer_num > 0) {
				printf("Received %d peers from server\n", peer_num);
				net_add_peers(ptr + 35, peer_num);
			}

			/* Update status */
			network.status = 2;
			network.tout = 0;
			
			/* Run callback-function */
			if(network.on_success != NULL)
				network.on_success(NULL, 0);
		}	
	}
	else if(op == REQ_OP_LIST) {
		int num = ptr[0];
		if(num > 0)
			net_add_peers(ptr + 1, num);
	}
	else if(op == REQ_OP_CONVEY) {
		char res = ptr[0];

		/* Failed */
		if(res < 0) {
			int slot = net_peer_sel_id((uint32_t *)ptr + 1);

			/* Reset entry in peer-table */
			if(slot > 0)
				tbl->mask[slot] = PEER_M_NONE;
		}
		/* Check if connection can be established */
		else if(res == 1) {
			short slot_num = *(short *)(ptr + 1);
			uint32_t id = *(uint32_t *)(ptr + 3);

			int ti_mod = time(NULL) % 4096;
			int size = 0;

			tmp = REQ_HDR_SIZEW;
			ptr = pck + tmp;	

			/* If possible accept request */
			if(tbl->con_num + tbl->pen_num < PEER_CON_NUM) {
				int slot = lcp_get_slot(ctx);
				short port = ctx->sock.ext_port[slot];
				int n;

				ptr[1] = 1;
				memcpy(ptr + 4, &ctx->ext_addr, 16);
				memcpy(ptr + 20, &port, 2);
				ptr[22] = ctx->con_flg;

				size = 19;

				/* Add peer to peer-table */
				if((n = net_add_peer(&id)) < 0) {
					printf("ID not found!!!!!\n");
					ptr[1] = 0;
				}
				else {
					/* Set my own port */
					tbl->port[n] = port;

					printf("Slot %d\n", n);
				}

			}
			else {
				ptr[1] = 0;
			}

			/* Send response-header */
			hdr_set(pck, REQ_OP_CONVEY, 0x1, network.id,
					ti_mod, network.key);

			ptr[0] = 2;

			/* Copy slot-number */
			memcpy(ptr + 2, &slot_num, 2);

			/* Send packet */
			lcp_send(ctx, &network.main_addr, pck, tmp + 4 + size);
		}
		/* Establish new connection */
		else if(res == 3) {
			uint32_t id;
			struct sockaddr_in6 addr;
			char flg;
			uint16_t proxy_num;	
			int slot_num;
			struct lcp_con *con;
			unsigned short port;

			/* Get the peer-id */
			memcpy(&id, ptr + 1, 4);

			/* Copy the address */
			lcp_addr(&addr, ptr + 5, ptr + 21);

			/* Get the connection-flag */
			flg = ptr[23];

			/* Get the proxy-number */
			memcpy(&proxy_num, ptr + 24, 2);

			/* Get the slot in the peer-table */
			if((slot_num = net_peer_sel_id(&id)) < 0) {
				printf("ID not found\n");
				return -1;
			}

			/* Update slot-mask */
			tbl->status[slot_num] = PEER_S_PENDING;

			/* Set attributes */
			tbl->addr[slot_num] = addr;
			tbl->flag[slot_num] = flg;


			printf("Connect to port %d\n", tbl->addr[slot_num].sin6_port);

			/* Initate connection */
			port = tbl->port[slot_num];
			if(!(con = lcp_connect(ctx, port, &addr, flg, 0))) {
				tbl->mask[slot_num] = PEER_M_NONE;
				printf("Failed to contact port\n");
				return -1;
			}

			con->proxy_id = proxy_num;

			/* Set connection-pointer */
			tbl->con[slot_num] = con; 

			/* Adjust the number of pending-connections */
			tbl->pen_num++;
		}
	}

	return 0;
}


static int hashSHA256(void* input, unsigned long length, unsigned char *md)
{
	SHA256_CTX context;
	if(!SHA256_Init(&context))
		return -1;

	if(!SHA256_Update(&context, (unsigned char*)input, length))
		return -1;

	if(!SHA256_Final(md, &context))
		return -1;

	return 0;
}


extern int net_insert(char *uname, char *pswd, net_cfnc on_success, 
		net_cfnc on_failed)
{
	int tmp;
	uint8_t pswd_enc[32];
	char pck[512];

	/* Passed buffers are invalid */
	if(uname == NULL || pswd == NULL)
		return -1;

	/* If a request is already pending */
	if(network.status != 0)
		return -1;

	/* Check if username has a valid length */
	tmp = strlen(uname);
	if(tmp < 5 || tmp > 16)
		return -1;

	/* Update status and callback-functions */
	network.status = 1;
	network.on_success = on_success;
	network.on_failed = on_failed;

	/* Encrypt password */
	hashSHA256(pswd, strlen(pswd), pswd_enc);

	tmp = hdr_set(pck, REQ_OP_INSERT, 0x1, 0x0, 0, NULL);
	strcpy(pck + tmp, uname);
	memcpy(pck + tmp + strlen(uname) + 1, pswd_enc, 32);
	pck[tmp + strlen(uname) + 33] = network.ctx->con_flg; 

	/* Send request */
	return lcp_send(network.ctx, &network.main_addr, pck, 
			tmp + strlen(uname) + 34);
}


extern int net_list(net_cfnc on_success, net_cfnc on_failed)
{
	time_t ti;
	uint16_t ti_mod;

	char pck[512];

	if(on_success || on_failed) {/* Prev warning for not using params */}

	/* Get the time-modulator */
	time(&ti);
	ti_mod = ti % 4096;

	/* Send request to server */
	hdr_set(pck, REQ_OP_LIST, 0x01, network.id, ti_mod, network.key);
	return lcp_send(network.ctx, &network.main_addr, pck, REQ_HDR_SIZEW);
}


extern int net_add_peer(uint32_t *id)
{
	int i;
	struct peer_table *tbl = &network.peers;

	for(i = 0; i < PEER_SLOTS; i++) {
		if(tbl->mask[i] == PEER_M_NONE) {
			tbl->mask[i] = PEER_M_SET;
			tbl->status[i] = PEER_S_AWAIT;
			tbl->id[i] = *id;

			tbl->num++;
			return i;
		}
	}

	return -1;
}


extern int net_add_peers(char *buf, short num)
{
	short i;
	short j;
	int tmp;
	uint32_t id;
	char *ptr = buf;
	struct peer_table *tbl = &network.peers;

	for(i = 0; i < num; i++) {
		/* Copy peer-id */
		id = *(uint32_t *)ptr;

		printf("Insert peer %d with id %d\n", i, id);

		/* If the peer has the same id as this peer */
		if(id == network.id)
			continue;

		/* Check if the peer is already in the table */
		tmp = 0;
		for(j = 0; j < PEER_SLOTS; j++) {
			if(tbl->mask[j] != 0 && tbl->id[j] == id) {
				tmp = 1;
				break;
			}
		}

		if(tmp)
			continue;

		/* Try to insert peer into empty slot */
		for(j = 0; j < PEER_SLOTS; j++) {
			if(tbl->mask[j] == PEER_M_NONE) {
				tbl->mask[j] = PEER_M_SET;
				tbl->status[j] = PEER_S_SET;
				tbl->id[j] = id;

				tbl->num++;
				break;
			}
		}

		/* Update pointer */
		ptr += 4;
	}

	return 0;
}


extern short net_peer_sel_addr(struct in6_addr *addr, unsigned short *port)
{
	short i;
	struct peer_table *tbl = &network.peers;

	for(i = 0; i < PEER_SLOTS; i++) {
		if(tbl->mask[i] == PEER_M_NONE)
			continue;

		printf("%d - %d\n", *port, tbl->addr[i].sin6_port);
		if(memcmp(addr, &tbl->addr[i].sin6_addr, 16) == 0 && 
				memcmp(port, &tbl->addr[i].sin6_port, 2) == 0)
			return i;
	}

	return -1;
}


extern short net_peer_sel_id(uint32_t *id)
{
	short i;
	struct peer_table *tbl = &network.peers;

	for(i = 0; i < PEER_SLOTS; i++) {
		if(tbl->mask[i] == PEER_M_NONE)
			continue;

		if(tbl->id[i] == *id)
			return i;
	}

	return -1;
}


extern int net_con_peers(void)
{
	int i;
	struct peer_table *tbl = &network.peers;
	short slot;
	unsigned short port;
	char pck[256];
	int tmp;
	uint32_t ti;

	for(i = 0; i < PEER_SLOTS; i++) {
		if(tbl->con_num + tbl->pen_num >= PEER_CON_NUM)
			break;

		if(tbl->mask[i] == PEER_M_NONE)
			continue;

		/* If the peers are already connected */
		if(tbl->status[i] >= PEER_S_PENDING)
			continue;

		/* Get an external port to establish the connection with */
		if((slot = lcp_get_slot(network.ctx)) < 0)
			continue;

		printf("Connect to %d\n", tbl->id[i]);

		port = network.ctx->sock.ext_port[slot];

		/* Update mask */
		tbl->status[i] = PEER_S_AWAIT;

		/* Update number of pending connections */
		tbl->pen_num++;

		/* Set my own port */
		tbl->port[i] = port;

		/* Set header */
		ti = time(NULL) % 0xffffffff;
		tmp = hdr_set(pck, REQ_OP_CONVEY, 0x1, network.id, ti, 
				network.key);

		/* Fill in payload */
		pck[tmp] = 0;
		memcpy(pck + tmp + 1, &tbl->id[i], 4);
		memcpy(pck + tmp + 5, &network.ctx->ext_addr, 16);
		memcpy(pck + tmp + 21, &port, 2);
		pck[tmp + 23] = network.ctx->con_flg;

		/* Send packet */
		lcp_send(network.ctx, &network.main_addr, pck, tmp + 23);
	}

	return 0;
}
