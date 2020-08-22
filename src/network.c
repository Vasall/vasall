#include "network.h"
#include "error.h"

#include "core.h"
#define DEF_HEADER
#include "header.h"

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
	network.obj_lst = NULL;
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
	int size;

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


				/* Synchronize the object-tables */
				tmp = hdr_set(pck, HDR_OP_EXC, tbl->id[slot],
						network.id, network.key);

				/* Attach list of objects */
				size = obj_list(pck + tmp, NULL, 128);

				/* Send packet */
				lcp_send(network.ctx, &tbl->addr[slot], pck, tmp + size);
			}
		}
		/* Received a packet */
		else if(evt.type == LCP_RECEIVED) {
			/* Handle packets if they seem valid */
			if(evt.len >= HDR_SIZE)
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


extern int net_broadcast(uint16_t op, char *buf, int len)
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
		tmp = hdr_set(pck, op, tbl->id[slot], network.id, network.key);

		/* Send packet */
		/* TODO: Handle failed */
		lcp_send(network.ctx, &tbl->addr[slot], pck, tmp + len);
	}

	return 0;
}


static int peer_hdl_ins(struct req_hdr *hdr, struct lcp_evt *evt,
		char *ptr, int len)
{
	uint32_t id;
	uint32_t mask;
	float pos[3];
	int mdl;
	short slot;
	short peer_num;

	struct timeval serv_ti;
	struct timeval loc_ti;
	uint32_t tdel;

	if(hdr||evt||len){/* Prevent warning for not using parameters */}

	if(network.status == 1) {
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

		/* Copy the server timestamp */
		memcpy(&serv_ti.tv_sec, ptr + 21, 8);
		memcpy(&serv_ti.tv_usec, ptr + 29, 8);

		gettimeofday(&loc_ti, NULL);
		tdel = (loc_ti.tv_sec - serv_ti.tv_sec) * 1000;
		tdel += (loc_ti.tv_usec - serv_ti.tv_usec) / 1000;
		tdel -= net_gettime();

		network.time_del = floor(tdel / TICK_TIME) * TICK_TIME;

		/* Insert object into object-table */
		id = network.id;
		memcpy(pos, ptr + 37, 3 * sizeof(float));
		mask = OBJ_M_PLAYER;
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
		memcpy(&peer_num, ptr + 49, 2);
		if(peer_num > 0)
			net_add_peers(ptr + 51, peer_num);

		/* Update status */
		network.status = 2;
		network.tout = 0;

		/* Run callback-function */
		if(network.on_success != NULL)
			network.on_success(NULL, 0);
	}

	return 0;
}

static int peer_hdl_lst(struct req_hdr *hdr, struct lcp_evt *evt,
		char *ptr, int len)
{
	int num;

	if(hdr||evt||len){/* Prevent warning for not using parameters */}

	if((num = ptr[0]) > 0)
		return net_add_peers(ptr + 1, num);

	return 0;
}

static int peer_hdl_cvy(struct req_hdr *hdr, struct lcp_evt *evt,
		char *ptr, int len)
{
	char res = ptr[0];
	char pck[512];
	int tmp;
	struct lcp_ctx *ctx = network.ctx;
	struct peer_table *tbl = &network.peers;

	if(hdr||evt||len){/* Prevent warning for not using parameters */}

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

		tmp = HDR_SIZEW;
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
		hdr_set(pck, HDR_OP_CVY, 0x1, network.id, network.key);

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
		short p_slot;

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

	return 0;
}

static int peer_hdl_exc(struct req_hdr *hdr, struct lcp_evt *evt,
		char *ptr, int len)
{
	short num;
	char *buf;
	int written;
	int tmp;
	char pck[512];

	if(evt||len){/* Prevent warning for not using parameters */}

	memcpy(&num, ptr, 2);

	/* Insert object-ids into cache */
	written = net_obj_insert(ptr + 2, num, hdr->src_id, &buf, NULL);

	/* Send response-header */
	tmp = hdr_set(pck, HDR_OP_GET, hdr->src_id, network.id, network.key);

	/* Attach payload to the packet */
	memcpy(pck + tmp, buf, written);
	free(buf);

	/* Send the packet */
	lcp_send(network.ctx, &evt->addr, pck, tmp + written);

	return 0;
}

static int peer_hdl_get(struct req_hdr *hdr, struct lcp_evt *evt,
		char *ptr, int len)
{
	short num;
	void *obj_lst;
	int written;
	uint32_t ts;
	int tmp;
	char pck[512];
	uint16_t flg = (OBJ_A_ID|OBJ_A_MASK|OBJ_A_POS|OBJ_A_VEL|OBJ_A_MOV);

	if(len){/* Prevent warning for not using parameters */}

	memcpy(&num, ptr, 2);

	written = obj_collect(flg, ptr + 2, num, &obj_lst, NULL);

	/* Set response-header */
	tmp = hdr_set(pck, HDR_OP_SBM, hdr->src_id, network.id, network.key);

	/* Attach timestamp */
	ts = net_gettime();
	memcpy(pck + tmp, &ts, 4);

	/* Attach payload to packet */
	memcpy(pck + tmp + 4, obj_lst, written);
	free(obj_lst);

	/* Send the packet */
	lcp_send(network.ctx, &evt->addr, pck, tmp + written);

	return 0;
}

static int peer_hdl_sbm(struct req_hdr *hdr, struct lcp_evt *evt,
		char *ptr, int len)
{
	short num;
	uint32_t ts;

	if(evt||len){/* Prevent warning for not using parameters */}

	memcpy(&ts, ptr, 4);
	ts -= network.time_del;

	memcpy(&num, ptr + 4, 2);

	/* Submit list of objects */
	return net_obj_submit(ptr + 6, ts, num, hdr->src_id);
}

static int peer_hdl_upd(struct req_hdr *hdr, struct lcp_evt *evt,
		char *ptr, int len)
{
	uint32_t ts;

	if(hdr||evt||len){/* Prevent warning for not using parameters */}

	ts = *(uint32_t *)ptr;
	ts -= network.time_del;

	/* Update the object */
	return obj_add_inputs(ts, ptr + 4);
}

static int peer_hdl_syn(struct req_hdr *hdr, struct lcp_evt *evt,
		char *ptr, int len)
{
	if(hdr||evt||ptr||len){/* Prevent warning for not using parameters */}
	return 0;
}

extern int peer_handle(struct lcp_evt *evt)
{
	char *ptr;
	int tmp;
	int r = 0;
	struct req_hdr hdr;
	int len;

	/* Extract data from header */
	tmp = hdr_cpy(evt->buf, &hdr);
	len = evt->len - tmp;

	/* Set pointer */
	ptr = evt->buf + tmp;

	switch(hdr.op) {
		case HDR_OP_INS: r = peer_hdl_ins(&hdr, evt, ptr, len); break;
		case HDR_OP_RMV: break;
		case HDR_OP_VAL: break;
		case HDR_OP_LST: r = peer_hdl_lst(&hdr, evt, ptr, len); break;
		case HDR_OP_CVY: r = peer_hdl_cvy(&hdr, evt, ptr, len); break;
		case HDR_OP_EXC: r = peer_hdl_exc(&hdr, evt, ptr, len); break;
		case HDR_OP_GET: r = peer_hdl_get(&hdr, evt, ptr, len); break;
		case HDR_OP_SBM: r = peer_hdl_sbm(&hdr, evt, ptr, len); break;
		case HDR_OP_UPD: r = peer_hdl_upd(&hdr, evt, ptr, len); break;
		case HDR_OP_CMP: break;
		case HDR_OP_SYN: r = peer_hdl_syn(&hdr, evt, ptr, len); break;
	}

	return r;
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


extern int net_insert(char *uname, char *pswd, net_fnc on_success, 
		net_fnc on_failed)
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

	tmp = hdr_set(pck, HDR_OP_INS, 0x1, 0x0, NULL);
	strcpy(pck + tmp, uname);
	memcpy(pck + tmp + strlen(uname) + 1, pswd_enc, 32);
	pck[tmp + strlen(uname) + 33] = network.ctx->con_flg; 

	/* Send request */
	tmp = tmp + strlen(uname) + 34;
	return lcp_send(network.ctx, &network.main_addr, pck, tmp);
}


extern int net_list(net_fnc on_success, net_fnc on_failed)
{
	char pck[512];

	if(on_success || on_failed) {/* Prev warning for not using params */}

	/* Send request to server */
	hdr_set(pck, HDR_OP_LST, 0x01, network.id, network.key);
	return lcp_send(network.ctx, &network.main_addr, pck, HDR_SIZEW);
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

		if(tmp == 1)
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
		tmp = hdr_set(pck, HDR_OP_CVY, 0x1, network.id, network.key);

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
	if((cur = network.obj_lst) != NULL) {
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
				network.obj_lst = ent;
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
	struct cache_entry *ptr = network.obj_lst;

	while(ptr != NULL) {
		if(id == ptr->id)
			return ptr;

		ptr = ptr->next;
	}

	return NULL;
}


extern int net_obj_submit(void *ptr, uint32_t ts, short num, uint32_t src)
{
	short i;
	uint32_t obj_id;
	struct cache_entry *ent;
	struct cache_entry *prev;
	char *buf_ptr;
	short src_slot;

	/* Get the slot of the peer */
	if((src_slot = net_peer_sel_id(&src)) < 0)
		return -1;

	/* Set pointer to read object-data */
	buf_ptr = ptr;

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
				network.obj_lst = ent->next;
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


extern uint32_t net_gettime(void)
{
	return SDL_GetTicks() + network.time_del; 
}
