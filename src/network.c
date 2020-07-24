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
	struct sockaddr_in6 *addr;
	struct lcp_evt evt;
	char running = 1;
	short i;

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

#if 0
	/* Initialize the LCP-context */
	if(!(network.ctx = lcp_init(rand() % 3000 + 10000, 0, 0, &disco, &proxy)))
		return -1;
#else
	/* Initialize the LCP-context */
	if(!(network.ctx = lcp_init(rand() % 3000 + 10000, 0, LCP_NET_F_OPEN, &disco, &proxy)))
		return -1;
#endif

	/* Set initial values */
	network.status = 0;
	network.tout = 0;

	/* Initialize the peer-table */
	if(net_peer_init() < 0)
		goto err_close_ctx;

	/* Initialize connected peer-table */
	network.con_num = 0;
	for(i = 0; i < PEER_CON_NUM; i++)
		network.con[i] = -1;

	/* Initialize the object-cache */
	network.cache = NULL;
	network.count = 0;

	addr = &network.main_addr;
	printf("Connect to server: %s\n", lcp_str_addr6(addr));

	/* Connect to server */
	if(!(lcp_connect(network.ctx, -1, addr, LCP_CON_F_DIRECT, LCP_F_ENC)))
		goto err_close_ctx;

	while(running) {
		lcp_update(network.ctx);

		while(lcp_pull_evt(network.ctx, &evt)) {	
			if(evt.type == LCP_CONNECTED) {
				running = 0;
				break;
			}
			else if(evt.type == LCP_UNAVAILABLE) {
				ERR_LOG(("Failed to contact server"));
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
}


extern int net_update(void)
{
	struct lcp_evt evt;
	time_t ti;
	struct peer_table *tbl = &network.peers;

	/* Processing incoming packets and send out requests */
	lcp_update(network.ctx);

	while(lcp_pull_evt(network.ctx, &evt)) {
		/* Connected to a peer */
		if(evt.type == LCP_CONNECTED) {
			short slot;
			struct in6_addr addr;
			unsigned short port;

			memcpy(&addr, &evt.addr.sin6_addr, 16);
			port = evt.addr.sin6_port;

			/* Mark peer as connected */
			if((slot = net_peer_sel_addr(&addr, &port)) >= 0) {
				char pck[534];
				int tmp;
				struct timeval rt;
				uint32_t ts;

				/* Add peer to connected-list */
				net_con_add(slot);

				/* Update entry-mask */
				tbl->mask[slot] |= PEER_M_CON;
				tbl->status[slot] = PEER_S_CON;

				/* Update num of connected and pending peers */
				tbl->con_num++;
				tbl->pen_num--;

				printf("Connected to %s on slot %d\n", 
						lcp_str_addr6(&evt.addr), slot);

				/* Send exchange packet to peer */
				tmp = hdr_set(pck, REQ_OP_EXCHANGE,
						tbl->id[slot], network.id, 
						network.key);

				memset(pck + tmp, 0, 20);

				/* Get current real-time and timestamp */
				gettimeofday(&rt, NULL);
				ts = SDL_GetTicks();

				/* Attach real-time */
				memcpy(pck + tmp, &rt.tv_sec, 8);
				memcpy(pck + tmp + 8, &rt.tv_usec, 8);

				/* Attach timestamp */
				memcpy(pck + tmp + 16, &ts, 4);

				/* Send packet to peer */
				lcp_send(network.ctx, &tbl->addr[slot], pck, tmp + 20);
			}
		}
		/* Received a packet */
		else if(evt.type == LCP_RECEIVED) {
			/* Handle packets if they seem valid */
			if(evt.len >= REQ_HDR_SIZE)
				peer_handle(&evt);
		}
		/* Failed to deliver a packet */
		else if(evt.type == LCP_FAILED) {

		}
		/* Peer timed out */
		else if(evt.type == LCP_TIMEDOUT) {
			short slot;
			struct in6_addr addr;
			unsigned short port;

			memcpy(&addr, &evt.addr.sin6_addr, 16);
			port = evt.addr.sin6_port;

			if((slot = net_peer_sel_addr(&addr, &port)) >= 0) {
				/* Remove peer from connected list */
				net_con_remv(slot);

				/* Remove peer from the peer-table */
				tbl->mask[slot] = PEER_M_NONE;
			}
		}

		lcp_del_evt(&evt);
	}

	time(&ti);

	if(network.status == 0x02) {
		int num = tbl->con_num + tbl->pen_num;

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
	}

	return 0;
}


extern int net_broadcast(char *buf, int len)
{
	char pck[532];
	int tmp;
	short i;
	short slot;
	struct peer_table *tbl = &network.peers;

	/* Copy buffer into packet */
	memcpy(pck + 20, buf, len);

	for(i = 0; i < PEER_CON_NUM; i++) {
		if(network.con[i] == -1)
			continue;

		/* Set header of packet */
		slot = network.con[i];
		tmp = hdr_set(pck, REQ_OP_UPDATE, tbl->id[slot], 
				network.id, network.key);

		/* Send packet */
		/* TODO: Handle failed */
		lcp_send(network.ctx, &tbl->addr[slot], pck, tmp + len);
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

	short p_slot;

	/* Extract data from header */
	tmp = hdr_get(evt->buf, &op, &dst_id, &src_id, &mod, &key);
	len = evt->len - tmp;

	if(len) {/* Prevent warning for not using len yet */}

	/* Set pointer */
	ptr = evt->buf + tmp;

	/* Get the slot in the peer-table */
	p_slot = net_peer_sel_id(&src_id);


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
			mask = OBJ_M_PLAYER;
			mdl = mdl_get("plr");

			if((slot = obj_set(id, mask, pos, mdl, NULL, 0)) < 0) {
				/* Run callback-function */
				if(network.on_failed != NULL)
					network.on_failed(NULL, 0);

				return 0;
			}

			/* Set the index of the core-object */
			core.obj[0] = slot;

			/* Check if any peers are includes */
			memcpy(&peer_num, ptr + 33, 2);
			if(peer_num > 0)
				net_add_peers(ptr + 35, peer_num);

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
				if((n = net_add_peer(&id)) >= 0)
					tbl->port[n] = port;
				else
					ptr[1] = 0;

			}
			else {
				ptr[1] = 0;
			}

			/* Send response-header */
			hdr_set(pck, REQ_OP_CONVEY, 0x1, network.id,
					network.key);

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
			struct lcp_con *con;
			unsigned short port;

			/* Get the peer-id */
			memcpy(&id, ptr + 1, 4);

			/* Get the peer-slot */
			p_slot = net_peer_sel_id(&id);

			/* Copy the address */
			lcp_addr(&addr, ptr + 5, ptr + 21);

			/* Get the connection-flag */
			flg = ptr[23];

			/* Get the proxy-number */
			memcpy(&proxy_num, ptr + 24, 2);

			/* Get the slot in the peer-table */
			if(p_slot  < 0)
				return -1;

			/* Update slot-mask */
			tbl->status[p_slot] = PEER_S_PEN;

			/* Set attributes */
			tbl->addr[p_slot] = addr;
			tbl->flag[p_slot] = flg;

			/* Initiate connection */
			port = tbl->port[p_slot];
			if(!(con = lcp_connect(ctx, port, &addr, flg, 0))) {
				tbl->mask[p_slot] = PEER_M_NONE;
				return -1;
			}

			/* Set the proxy-id of the connection */
			con->proxy_id = proxy_num;

			/* Set connection-pointer */
			tbl->con[p_slot] = con; 

			/* Adjust the number of pending-connections */
			tbl->pen_num++;
		}
	}
	else if(op == REQ_OP_EXCHANGE) {
		long int p_sec;
		long int p_usec;
		long int p_ts = 0;
		struct timeval rt;
		long int ts = 0;
		int size;
		long int del;

		/* TODO: Rework the timestamp exchange */

		/* Extract data from packet */
		memcpy(&p_sec, ptr, 8);
		memcpy(&p_usec, ptr + 8, 8);
		memcpy(&p_ts, ptr + 16, 4);

		/* Get current real-time and the current timestamp */
		gettimeofday(&rt, NULL);
		ts = SDL_GetTicks();

		/* Get the relative time-difference to set timestamp back */
		del = (rt.tv_sec - p_sec) * 1000;
		del += (rt.tv_usec / 1000) - (p_usec / 1000);
		del = (ts - del) - p_ts;	

		/* Insert timedelta into peer-table */
		tbl->ti_del[p_slot] = del;
		printf("Timedelta of %ld milliseconds\n", tbl->ti_del[p_slot]);
			
		/* Update status */
		tbl->status[p_slot] = PEER_S_EXC; 

		/* Synchronize the object-tables */
		tmp = hdr_set(pck, REQ_OP_SYNC, src_id, network.id,
				network.key);

		/* Attach list of objects */
		size = obj_list(pck + tmp, NULL, 128);	

		/* Send packet */
		lcp_send(network.ctx, &tbl->addr[p_slot], pck, tmp + size);
	}
	else if(op == REQ_OP_SYNC) {
		short num;
		char *buf;
		int written;

		memcpy(&num, ptr, 2);

		/* Insert object-ids into cache */
		written = net_obj_insert(ptr + 2, num, src_id, &buf, NULL);

		/* Send response-header */
		tmp = hdr_set(pck, REQ_OP_REQUEST, src_id, network.id,
					network.key);

		/* Attach payload to the packet */
		memcpy(pck + tmp, buf, written);
		free(buf);

		/* Send the packet */
		lcp_send(network.ctx, &evt->addr, pck, tmp + written);
	}
	else if(op == REQ_OP_REQUEST) {
		short num;
		void *obj_buf;
		int written;

		memcpy(&num, ptr, 2);

		written = obj_collect(ptr + 2, num, &obj_buf, NULL);

		/* Set response-header */
		tmp = hdr_set(pck, REQ_OP_SUBMIT, src_id, network.id,
					network.key);

		/* Attach payload to packet */
		memcpy(pck + tmp, obj_buf, written);
		free(obj_buf);

		/* Send the packet */
		lcp_send(network.ctx, &evt->addr, pck, tmp + written);
	}
	else if(op == REQ_OP_SUBMIT) {
		/* Submit list of objects */
		net_obj_submit(ptr + 2, *(short *)ptr, src_id);
	}
	else if(op == REQ_OP_UPDATE) {
		uint32_t ti;

		memcpy(&ti, ptr, 4);
		ti += tbl->ti_del[p_slot];
		memcpy(ptr, &ti, 4);

		/* Update object-movement and action */
		obj_update(ptr);
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

	tmp = hdr_set(pck, REQ_OP_INSERT, 0x1, 0x0, NULL);
	strcpy(pck + tmp, uname);
	memcpy(pck + tmp + strlen(uname) + 1, pswd_enc, 32);
	pck[tmp + strlen(uname) + 33] = network.ctx->con_flg; 

	/* Send request */
	return lcp_send(network.ctx, &network.main_addr, pck, 
			tmp + strlen(uname) + 34);
}


extern int net_list(net_cfnc on_success, net_cfnc on_failed)
{
	char pck[512];

	if(on_success || on_failed) {/* Prev warning for not using params */}

	/* Send request to server */
	hdr_set(pck, REQ_OP_LIST, 0x01, network.id, network.key);
	return lcp_send(network.ctx, &network.main_addr, pck, REQ_HDR_SIZEW);
}


extern int net_add_peer(uint32_t *id)
{
	int i;
	struct peer_table *tbl = &network.peers;

	/* Check if the peer is already in the table */
	for(i = 0; i < PEER_SLOTS; i++) {
		if(tbl->mask[i] != 0 && tbl->id[i] == *id)
			return -1;
	}

	for(i = 0; i < PEER_SLOTS; i++) {
		if(tbl->mask[i] == PEER_M_NONE) {
			tbl->mask[i] = PEER_M_SET;
			tbl->status[i] = PEER_S_AWA;
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

	for(i = 0; i < PEER_SLOTS; i++) {
		if(tbl->con_num + tbl->pen_num >= PEER_CON_NUM)
			break;

		if(tbl->mask[i] == PEER_M_NONE)
			continue;

		/* If the connection is pending or established */
		if(tbl->status[i] >= PEER_S_PEN)
			continue;

		/* Get an external port to establish the connection with */
		if((slot = lcp_get_slot(network.ctx)) < 0)
			continue;

		/* Get the external-port of the socket */
		port = network.ctx->sock.ext_port[slot];

		/* Update mask */
		tbl->status[i] = PEER_S_AWA;

		/* Update number of pending connections */
		tbl->pen_num++;

		/* Set my own port */
		tbl->port[i] = port;

		/* Set header */
		tmp = hdr_set(pck, REQ_OP_CONVEY, 0x1, network.id, network.key);

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


extern int net_con_add(short slot)
{
	int i;

	if(network.con_num >= PEER_CON_NUM)
		return -1;

	for(i = 0; i < PEER_CON_NUM; i++) {
		if(network.con[i] == -1) {
			network.con[i] = slot;
			network.con_num++;
			return i;
		}
	}

	return -1;
}


extern void net_con_remv(short slot)
{
	short i;

	for(i = 0; i < PEER_CON_NUM; i++) {
		if(network.con[i] == slot) {
			network.con[i] = -1;
			network.con_num--;
			break;
		}
	}
}


extern int net_obj_insert(void *in, short in_num, uint32_t src, char **out,
		short *out_num)
{
	int i;
	struct cache_entry *cur;
	struct cache_entry *ent;
	uint32_t id;
	uint32_t *id_ptr = (uint32_t *)in;

	char *out_buf;
	uint32_t *ptr;
	uint32_t num = 0;

	int written = 0;

	if(in == NULL || in_num <= 0 || out == NULL)
		return -1;

	if(!(out_buf = malloc(in_num * 4 + 2)))
		return -1;

	ptr = (uint32_t *)(out_buf + 2);

	/* Get the tail of the linked-list */
	if((cur = network.cache) != NULL) {
		while(cur->next != NULL)
			cur = cur->next;
	}

	for(i = 0; i < in_num; i++) {
		id = *id_ptr;

		/* An object with the id is not yet registered or cached */
		if(obj_sel_id(id) < 0 && net_obj_find(id) == NULL) {
			/* Allocate memory for the struct */
			if(!(ent = malloc(sizeof(struct cache_entry))))
				return -1;

			/* Setup the struct */
			ent->next = NULL;
			ent->prev = cur;
			ent->id = id;
			ent->src = src;

			ent->status = 0;
			ent->tout = 0;

			/* Insert the entry into the linked-list */
			if(cur == NULL)
				network.cache = ent;
			else
				cur->next = ent;

			/* Update tail-pointer */
			cur = ent;

			/* Add object to the request-list */
			memcpy(ptr, &id, 4);
			num++;
			written += 4;
			ptr++;
		}

		/* Update id-pointer to the next id*/
		id_ptr++;
	}

	/* Copy number to buffer */
	memcpy(out_buf, &num, 2);

	/* Set the output-variables */
	*out = out_buf;
	if(out_num != NULL) *out_num = num;
	return written + 2;
}


extern struct cache_entry *net_obj_find(uint32_t id)
{
	struct cache_entry *ptr = network.cache;

	while(ptr != NULL) {
		if(id == ptr->id)
			return ptr;

		ptr = ptr->next;
	}

	return NULL;
}


extern int net_obj_submit(void *ptr, short num, uint32_t src)
{
	short i;
	uint32_t obj_id;
	struct cache_entry *ent;
	struct cache_entry *prev;
	char *buf_ptr;
	short src_slot;
	int64_t ts;

	/* Get the slot of the peer */
	if((src_slot = net_peer_sel_id(&src)) < 0)
		return -1;

	/* Extract data from packet */
	memcpy(&ts, ptr + 2, 4);

	/* Adjust timestamp */
	ts += network.peers.ti_del[src_slot];

	/* Set pointer to read object-data */
	buf_ptr = ptr + 4;

	for(i = 0; i < num; i++) {
		/* Get the id of the object */
		memcpy(&obj_id, buf_ptr, 4);

		/* Get the object from the object-cache */
		if((ent = net_obj_find(obj_id)) != NULL) {
			if(ent->src != src)
				continue;

			/* Push the objects into the object-table */
			obj_submit(buf_ptr, ts);

			/* Remove the object from the object-cache */
			if((prev = ent->prev) == NULL)
				network.cache = ent->next;
			else
				prev->next = ent->next;

			/* Free the allocated memory */
			free(ent);
		}

		/* Go to the next object */
		buf_ptr += 52;
	}

	return 0;
}
