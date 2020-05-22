#ifndef _UPNP_H
#define _UPNP_H

#include "miniupnpc/miniupnpc.h"
#include "miniupnpc/upnpcommands.h"

struct upnp_handle {
	struct UPNPUrls urls;
	struct IGDdatas data;
	struct UPNPDev *dev;
	char addr[64];
};


/*
 * Disover all uPnP devices and find a valid IGD. Then attach necessary data
 * to the context.
 *
 * @hdl: Pointer to the uPnP-handle
 *
 * Returns: 0 on success or -1 if an error occurred
 */
extern int upnp_prep(struct upnp_handle *hdl);


/*
 * Free the necessary data and detach it from the context. 
 *
 * @hdl: Pointer to the uPnP-handle
 */
extern void upnp_close(struct upnp_handle *hdl);


/*
 * Add a new forward-entry in the NAT-table.
 *
 * @hdl: Pointer to the uPnP-handle
 * @in: The internal port
 * @ex: The external port
 *
 * Returns: 0 on success or -1 if an error occurred
 */
extern int upnp_add(struct upnp_handle *hdl, unsigned short in,
		unsigned int ex);


/*
 * Remove a forward-entry from the NAT-table.
 *
 * @hdl: Pointer to the uPnP-handle
 * @ex: The external port to free
 */
extern void upnp_remv(struct upnp_handle *hdl, unsigned short ex);

#endif
