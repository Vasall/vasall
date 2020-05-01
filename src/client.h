#ifndef _V_CLIENT_H
#define _V_CLIENT_H

struct cli_hdl;
struct cd_evt;
struct cd_evt_ent;
struct cd_evt_que;
struct cli_serv_req;

#include "defines.h"

#include "lcp/lcp.h"

#define REQ_STO 0x01
#define REQ_DTO 0x02
#define REQ_STO_R 0x11
#define REQ_DTO_R 0x12

struct cli_tok {
	uint32_t id;
	uint32_t key;
};

struct cli_hdl {
	struct lcp_sock *sock;

	struct in6_addr addr;
	short port;

	char uname[17];
	uint8_t rank;

	struct cli_tok tok;

	struct cli_serv_que *serv_que;

	struct cli_evt_que *evt_que;
};

V_GLOBAL struct cli_hdl *client;

/*
 * Create and initialize a new clusterd-client-instance. The instance contains 
 * the socket and client-token. Before exiting the programm, remeber to
 * always call cli_close() to clear everything up.
 *
 * @addr: The server-address to connect with
 * @port: The server-port to conntact
 * 
 * Returns: Either 0 on success or -1 if an error occurred
 */
V_API int cli_init(char *addr, short port);

/*
 * Close the client-instance and free the allocated memory.
 */
V_API void cli_close(void);

/*
 * This function will process received packets and handle them accordingly.
 * To do so the function has to be inside the main update-loop of the 
 * programm.
 */
V_API void cli_update(void);

struct cli_serv_req {
	struct cli_serv_req *next;

	char *buf;
	int len;
};

struct cli_serv_que {
	struct lcp_stm *stm;
	struct cli_serv_req *root;
};

V_API int cli_serv_init(void);

V_API void cli_serv_close(void);

V_API int cli_serv_push(char *buf, int len);

V_API void cli_serv_update(void);

/*
 * Request a token from the server using a username and password, which have
 * to have been registered on the server already. Requesting a token will
 * insert the client into the network. Note that this function will block
 * further progression until a key has been aquired or the connection has been
 * reset by the server.
 *
 * @uname: A null-terminated buffer containing the username
 * @pswd: A null-terminated buffer containing the password
 *
 * Returns: 1 on success, 0 if the action failed and -1 if an error occurred
 */
V_API int cli_sto(char *uname, char *pswd);

/*
 * Delete the token attached to the client and tell the server to delete it. 
 * This will effectively remove the client from the cluster. Not that this
 * function is blocking further progression until the request has been
 * processed.
 *
 * Returns: Either 1 on success or -1 if an error occurred
 */
V_API int cli_dto(void);

#define EVT_PCK 0x01
#define EVT_EST 0x02

struct cli_evt {
	uint8_t type;
	char *buf;
	int len;	
};

struct cli_evt_ent {
	struct cli_evt_ent *next;
	struct cli_evt evt;
};

struct cli_evt_que {
	struct cli_evt_ent *root;
};

/*
 * Create and initialize a new event-queue for the cluster-daemon-client.
 *
 * Returns: Either a pointer to the created struct or NULL if an error occured
 */
V_API int cli_evt_init(void);

/*
 * Close the client-event-queue and free the allocated memory.
 */
V_API void cli_evt_close(void);

/*
 * Pull an event from the event-queue and write it to the given struct. Please
 * note that to prevent memory leaks, you still have to call cd_evt_del() with
 * the event-struct to free the attached buffer.
 *
 * @evt: A pointer to write the event-struct to
 *
 * Returns: Either 1 if an event has been returned, 0 if the queue is empty and
 * 	-1 if an error occurred
 */
V_API int cli_evt_pull(struct cli_evt *evt); 

/*
 * Push an new entry into the event-queue. You can free the buffer after 
 * passing it to the function, as it will be copied into an extra buffer.
 *
 * @type: The type of the event
 * @buf: A buffer to attach to the event
 * @len: The length of the buffer in bytes
 *
 * Returns: 0 on success and -1 if an error occurred
 */
V_API int cli_evt_push(uint8_t type, char *buf, int len);

/*
 * This function will free the attached buffer and prevent memory leaks.
 * Therefore always call after having pulled an event from the event-queue,
 * even if the buffer is NULL.
 *
 * @evt: A pointer to the event-struct
 *
 * Returns: Either 0 on success or -1 if an error occurred
 */
V_API int cli_evt_del(struct cli_evt *evt);

#endif
