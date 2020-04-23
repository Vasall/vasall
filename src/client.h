#ifndef _CLIENT_H
#define _CLIENT_H

#include "clusterd-client/core.h"

struct client_handler {
	struct cd_core *core;
};

extern struct client_handler *client;

/*
 * Initialize the client, which will the be using for handling communication
 * between the client with the server and other clients.
 *
 * Returns: Either 0 on success or -1 if an error occurred
 */
int cli_init(char *addr, short port, short self);

/*
 * Close the client and free the allocated memory. If NULL if passed, the
 * function will just return.
 */
void cli_close(void);

/*
 * Update the client-struct and handle incoming packets.
 */
void cli_update(void);

#endif
