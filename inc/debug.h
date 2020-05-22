#ifndef _DEBUG_H
#define _DEBUG_H

/* 
 * Print an error-message in the console. 
 * Example: DEBUG_ERR(("message %d\n", var));
 */
#define DEBUG_ERR(x) do { if (DEBUG) dbg_printf x; } while (0)

#endif
