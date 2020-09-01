#ifndef _SETUP_H
#define _SETUP_H

#include "core.h"
#include "update.h"

extern void test1(char *buf, int len);
extern void test2(char *buf, int len);
extern void login(ui_node *n, event_t *e);

/*
 * Load all resources and add them to the dedicated asset-tables.
 */
int load_resources(void);


/*
 * Set the userinterface.
 */
int load_ui(void);

#endif
