#ifndef _V_UTILS_H_
#define _V_UTILS_H_

#include <stdint.h>

#ifdef __unix__
#include <byteswap.h>
#endif

/* 
#ifndef bswap_16
uint16_t __bswap_16(uint16_t val)
{
    return (val << 8) | (val >> 8 );
}

uint32_t __bswap_32(uint32_t val)
{
    val = ((val << 8) & 0xFF00FF00 ) | ((val >> 8) & 0xFF00FF ); 
    return (val << 16) | (val >> 16);
}
#endif
*/

/* Read a file into a buffer */
int readFile(char *pth, uint8_t **buf, long *len);

#endif
