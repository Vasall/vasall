#include "client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Redefine external variables */
struct client_handler *client = NULL;


V_API int cli_init(char *addr, short port, short self)
{
	if(client)
		return -1;

	if(!(client = malloc(sizeof(struct client_handler))))
		return -1;

	if(!(client->core = cd_init(addr, port, self)))
		goto err_free_client;

	return 0;

err_free_client:
	free(client);
	client = NULL;
	return -1;
}

V_API void cli_close(void)
{
	if(!client)
		return;

	cd_close(client->core);
	free(client);
	client = NULL;
}

V_API void cli_update(void)
{
	struct cd_evt evt;
	int i;
	uint32_t id;

	if(!client)
		return;

	cd_update(client->core);


	while(cd_evt_pull(client->core->evt_que, &evt)) {
		switch(evt.type) {
			case(CD_EVT_PCK):
				printf("Received %d:\n", evt.len);
				for(i = 0; i < evt.len; i++) {
					printf("%02x ", evt.buf[i]);

					if(i % 16 == 0 && i)
						printf("\n");
				}
				break;

			case(CD_EVT_EST):
				id = *(uint32_t *)evt.buf;
				printf("Connection with %08x\n", id);
				break;
		}

		cd_evt_del(&evt);
	}
}
