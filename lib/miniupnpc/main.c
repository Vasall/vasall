#include "miniupnpc.h"
#include "upnpcommands.h"
#include "upnperrors.h"
#include <stdio.h>

int main(void)
{
	struct UPNPUrls upnp_urls;
	struct IGDdatas upnp_data;
	struct UPNPDev *upnp_dev = 0;
	char lan_addr[64];
	char port[5] = "4242\0";
	int err = 0;

	/* Discover all uPnP devices */
	if(!(upnp_dev = upnpDiscover(2000, NULL, NULL, 0, 0, 2, &err))) {
		printf("Could not discover uPnP device(%d)\n", err);
		goto err_free_dev;
	}


	/* Retrieve a valid Internet Gateway Device */
	if((UPNP_GetValidIGD(upnp_dev, &upnp_urls, &upnp_data, lan_addr, 64)) != 1) {
		printf("No valid IGD found\n");
		goto err_cleanup;
	}


	printf("lan_addr=%s\n", lan_addr);
	printf("found valid IGD: %s\n", upnp_urls.controlURL);

	err = UPNP_AddPortMapping(
			upnp_urls.controlURL, 
			upnp_data.first.servicetype,
			/* external port */
			port,
			/* internal port */
			port,
			lan_addr, 
			"My Application Name", 
			"UDP",
			/* remote host */
			0,
			/* lease duration, recommended 0 as some NAT 
			 * implementations may not support another value */
			"0"
			);

	if(err) {
		printf("Failed to map port\n");
		printf("error: %s\n", strupnperror(err));
		goto err_cleanup;
	}
	
	
	printf("successfully mapped port\n");

err_cleanup:
	if((UPNP_DeletePortMapping(upnp_urls.controlURL, 
					upnp_data.first.servicetype, port, "UDP", 0)) != 0)
		printf("port map deletion error: %s\n", strupnperror(err));

	FreeUPNPUrls(&upnp_urls);

err_free_dev:
	freeUPNPDevlist(upnp_dev);
	return 0;
}
