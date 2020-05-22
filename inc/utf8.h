#ifndef _UTF8_H
#define _UTF8_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/*
 * Basic UTF-8 manipulation routines
 * by Jeff Bezanson
 * placed in the public domain Fall 2005
 *
 * This code is designed to provide the utilities you need to manipulate
 * UTF-8 as an internal string encoding. These functions do not perform the
 * error checking normally needed when handling UTF-8 data, so if you happen
 * to be from the Unicode Consortium you will want to flay me alive.
 * I do this because error checking can be performed at the boundaries (I/O),
 * with these routines reserved for higher performance on data known to be
 * valid.
 */

/* Is c the start of a utf8 sequence? */
#define isutf(c) (((c) & 0xC0) != 0x80)

static const uint32_t offsetsFromUTF8[6] = {
	0x00000000UL, 0x00003080UL, 0x000E2080UL,
	0x03C82080UL, 0xFA082080UL, 0x82082080UL
};

static const char trailingBytesForUTF8[256] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};


int u8_toucs(uint32_t *dest, int sz, char *src, int srcsz);
int u8_toutf8(char *dest, int sz, uint32_t *src, int srcsz);
int u8_wc_toutf8(char *dest, uint32_t ch);
int u8_offset(char *str, int charnum);
int u8_charnum(char *s, int offset);
uint32_t u8_nextchar(char *s, int *i);
void u8_inc(char *s, int *i);
void u8_dec(char *s, int *i);
int u8_seqlen(char *s);
int u8_read_escape_sequence(char *src, uint32_t *dest);
int u8_escape_wchar(char *buf, int sz, uint32_t ch);
int u8_unescape(char *buf, int sz, char *src);
int u8_escape(char *buf, int sz, char *src, int escape_quotes);
char *u8_strchr(char *s, uint32_t ch, int *charn);
char *u8_memchr(char *s, uint32_t ch, size_t sz, int *charn);
int u8_strlen(char *s);
int u8_is_locale_utf8(char *locale);
int u8_strins(char *s, char *i, int p);
int u8_strdel(char *s, int p, int l);

#endif
