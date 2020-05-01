#include "client.h"
#include "utils.h"
#include "hashtable.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/sha.h>

/* Redefine external variables */
struct cli_hdl *client = NULL;


V_INTERN int cli_serv_hdr(char *buf, uint8_t t, uint8_t kt, uint32_t id, 
		uint32_t key, short len);



V_API int cli_init(char *addr, short port)
{
	printf("Allocate core\n");
	if(!(client = malloc(sizeof(struct cli_hdl))))
		return -1;

	client->rank = 0;
	client->tok.id = 0;
	client->tok.key = 0;

	inet_pton(AF_INET6, addr, &client->addr);
	client->port = port;

	if(cli_serv_init() < 0)
		goto err_free_client;

	if(cli_evt_init() < 0)
		goto err_free_serv_que;

	if(!(client->sock = lcp_socket()))
		goto err_free_evt_que;

	return 0;

err_free_evt_que:
	cli_evt_close();
	
err_free_serv_que:
	cli_serv_close();

err_free_client:
	free(client);
	client = NULL;
	return -1;
}

V_API void cli_close(void)
{
	if(!client)
		return;

	lcp_close(client->sock);
	cli_evt_close();
	cli_serv_close();
	free(client);
}

V_API void cli_update(void)
{
	struct lcp_evt evt;
	uint8_t flg;

	if(!client)
		return;

	lcp_listen(client->sock);

	while(lcp_pull_evt(client->sock, &evt)) {
		switch(evt.type) {
			case(LCP_EVT_PSH):
				flg = *evt.buf;
				cli_evt_push(EVT_PCK, evt.buf, evt.len);
				break;
		}
	}

	cli_serv_update();
}

V_API int cli_serv_init(void)
{
	struct cli_serv_que *que;

	if(!client)	
		return -1;

	if(!(que = malloc(sizeof(struct cli_serv_que))))
		return -1;

	que->stm = NULL;
	que->root = NULL;
	client->serv_que = que;
	return 0;	
}

V_API void cli_serv_close(void)
{
	struct cli_serv_que *que;

	if(!client)
		return;

	if(!(que = client->serv_que))
		return;

	if(que->stm) {
		lcp_disconnect(que->stm);
	}

	if(que->root) {
		struct cli_serv_req *ptr = que->root;
		struct cli_serv_req *next;
		while(ptr) {
			next = ptr->next;
			free(ptr->buf);
			free(ptr);
			ptr = next;
		}
	}

	free(que);
}

V_API int cli_serv_push(char *buf, int len)
{
	struct cli_serv_que *que;
	struct cli_serv_req *req;

	if(!client || !buf || len <= 0)
		return -1;

	if(!(que = client->serv_que))
		return -1;

	if(!(req = malloc(sizeof(struct cli_serv_req))))
		return -1;

	req->next = NULL;
	req->buf = buf;
	req->len = len;

	if(!que->root) {
		que->root = req;
		return 0;
	}
	else {
		struct cli_serv_req *ptr = que->root;

		while(ptr) {
			if(!ptr->next) {
				ptr->next = req;
				return 0;
			}

			ptr = ptr->next;
		}
	}

	free(req->buf);
	free(req);
	return -1;
}

V_API void cli_serv_update(void)
{
	struct cli_serv_que *que;

	if(!client || !client->serv_que)
		return;

	que = client->serv_que;

	if(que->root) {
		if(que->stm) {
			if(que->stm->status == 1) {
				struct cli_serv_req *ptr;
				struct cli_serv_req *next;

				ptr = que->root;
				while(ptr) {
					next = ptr->next;

					lcp_send(client->sock, que->stm->slot, 
							ptr->buf, ptr->len, 
							LCP_F_PSH, 0);

					free(ptr->buf);
					free(ptr);

					ptr = next;
				}
				que->root = NULL;

			}
		}
		else {
			if(!(que->stm = lcp_connect(client->sock, 
							client->addr.s6_addr, 
							client->port, 0)))
				return;
		}
	}
	else {
		if(que->stm) {
			lcp_disconnect(que->stm);
			que->stm = NULL;
		}
	}
}

V_API int cli_sto(char *uname, char *pswd)
{
	int run = 1;
	char pck[51];
	short pck_len = 0;
	unsigned char pswd_enc[20];
	struct lcp_evt evt;
	int ret = 0;

	if(!client || !uname || !pswd)
		return -1;

	cli_serv_hdr(pck, 1, 0, 0, 0, strlen(uname) + 21);

	/* Attach username */
	strcpy(pck + 14, uname);

	/* Attach password */
	SHA1((unsigned char *)pswd, strlen(pswd), pswd_enc);
	memcpy(pck + 15 + strlen(uname), pswd_enc, 20);

	pck_len = strlen(uname) + 35;

	if(!lcp_connect(client->sock, client->addr.s6_addr, client->port, 
				LCP_O_SEC))
		return -1;

	while(run) {
		lcp_listen(client->sock);

		while(lcp_pull_evt(client->sock, &evt)) {	
			if(evt.type == LCP_EVT_INI) {
				lcp_send(client->sock, evt.stm->slot, pck, 
						pck_len, LCP_F_PSH, 0);
			}
			else if(evt.type == LCP_EVT_FIN) {
				run = 0;
				break;
			}
			else if(evt.type == LCP_EVT_PSH) {
				ret = *(evt.buf + 1);
				if(ret == 1) {
					memcpy(&client->tok.id, evt.buf + 2, 4);
					memcpy(&client->tok.key, evt.buf + 6, 4);
				}
				lcp_disconnect(evt.stm);
			}
		}
	}

	return ret;
}

V_API int cli_dto(void)
{
	char pck[14];
	short pck_len = 14;
	int run = 1;
	struct lcp_evt evt;

	cli_serv_hdr(pck, REQ_DTO, 1, client->tok.id, 
			client->tok.key, 0);

	if(!lcp_connect(client->sock, client->addr.s6_addr, client->port, 0))
		return -1;

	while(run) {
		lcp_listen(client->sock);

		while(lcp_pull_evt(client->sock, &evt)) {	
			if(evt.type == LCP_EVT_INI) {
				lcp_send(client->sock, evt.stm->slot, pck, 
						pck_len, LCP_F_PSH, 0);
			}
			else if(evt.type == LCP_EVT_FIN) {
				run = 0;
				break;
			}
			else if(evt.type == LCP_EVT_PSH) {
				lcp_disconnect(evt.stm);
			}
		}
	}

	client->tok.id = 0;
	client->tok.key = 0;
	return 1;
}

V_INTERN int cli_serv_hdr(char *buf, uint8_t t, uint8_t kt, uint32_t id, 
		uint32_t key, short len)
{
	time_t ti;
	short ti_mod;
	unsigned char key_buf[6];
	int key_hash = 0;

	memset(buf, 0, 14);

	buf[0] = t;
	buf[1] = kt;

	time(&ti);
	ti_mod = ti % 0xffff;
	memcpy(buf + 2, &ti_mod, 2);

	memcpy(buf + 4, &id, 4);

	memcpy(key_buf, &key, 4);
	memcpy(key_buf + 4, &ti_mod, 2);
	key_hash = hash((char *)key_buf, 6);
	memcpy(buf + 8, &key_hash, 4);

	memcpy(buf + 12, &len, 2);

	return 0;
}

V_API int cli_evt_init(void)
{
	struct cli_evt_que *que;

	if(!client)
		return -1;

	if(!(que = malloc(sizeof(struct cli_evt_que))))
		return -1;

	que->root = NULL;
	client->evt_que = que;
	return 0;
}

V_API void cli_evt_close(void)
{
	struct cli_evt_que *que;

	if(!client)
		return;

	if(!(que = client->evt_que))
		return;

	if(que->root) {
		struct cli_evt_ent *ptr;
		struct cli_evt_ent *next;

		ptr = que->root;
		while(ptr) {
			next = ptr->next;

			if(ptr->evt.buf)
				free(ptr->evt.buf);

			free(ptr);

			ptr = next;
		}

	}

	free(que);
}

V_API int cli_evt_pull(struct cli_evt *evt)
{
	struct cli_evt_que *que;
	struct cli_evt_ent *ent;

	if(!client)
		return 0;

	if(!(que = client->evt_que))
		return 0;

	ent = que->root;
	if(ent) {
		if(evt) {
			*evt = ent->evt;
		}
		else if(ent->evt.buf) {
			free(ent->evt.buf);
		}

		que->root = ent->next;
		free(ent);
		return 1;	
	}

	return 0;
}

V_API int cli_evt_push(uint8_t type, char *buf, int len)
{
	struct cli_evt_que *que;
	struct cli_evt_ent *ent;
	struct cli_evt_ent *ptr;

	if(!client)
		return -1;

	if(!(que = client->evt_que))
		return -1;

	if(!(ent = malloc(sizeof(struct cli_evt_ent))))
		return -1;

	ent->next = NULL;

	if(buf && len > 0) {
		if(!(ent->evt.buf = malloc(len)))
			goto err_free_ent;

		memcpy(ent->evt.buf, buf, len);
		ent->evt.len = len;
	}
	else {
		ent->evt.buf = NULL;
		ent->evt.len = 0;
	}

	ent->evt.type = type;

	if(!que->root) {
		que->root = ent;
		return 0;
	}
	
	ptr = que->root;
	while(ptr) {
		if(!ptr->next) {
			ptr->next = ent;
			return 0;
		}

		ptr = ptr->next;
	}

err_free_ent:
	free(ent);
	return -1;
}

V_API int cli_evt_del(struct cli_evt *evt)
{
	if(!evt)
		return -1;

	if(evt->buf)
		free(evt->buf);

	return 0;
}
