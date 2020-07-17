#include "utf8.h"

#include <string.h>
#include <stdarg.h>

static int octal_digit(char c)
{
	return (c >= '0' && c <= '7');
}

static int hex_digit(char c)
{
	return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') ||
			(c >= 'a' && c <= 'f');
}

#if !defined(HAVE_SNPRINTF) || defined(PREFER_PORTABLE_SNPRINTF)
#if defined(SOLARIS_BUG_COMPATIBLE) && !defined(SOLARIS_COMPATIBLE)
#define SOLARIS_COMPATIBLE
#endif

#if defined(HPUX_BUG_COMPATIBLE) && !defined(HPUX_COMPATIBLE)
#define HPUX_COMPATIBLE
#endif

#if defined(DIGITAL_UNIX_BUG_COMPATIBLE) && !defined(DIGITAL_UNIX_COMPATIBLE)
#define DIGITAL_UNIX_COMPATIBLE
#endif

#if defined(PERL_BUG_COMPATIBLE) && !defined(PERL_COMPATIBLE)
#define PERL_COMPATIBLE
#endif

#ifdef isdigit
#undef isdigit
#endif
#define isdigit(c) ((c) >= '0' && (c) <= '9')

#if !defined(HAVE_SNPRINTF)
/* declare our portable routine under name snprintf */
#define portable_snprintf snprintf
#else
/* declare our portable routine under name portable_snprintf */
#endif

/* *************************************************** */
/*                   PROTOTYPES                        */
/* *************************************************** */

static int u8_snprintf(char *str, size_t str_m, const char *fmt, ...) {
	va_list ap;
	size_t str_l = 0;
	const char *p = fmt;

	if(str_m < 1)
		return -1;

	va_start(ap, fmt);
	if(!p) p = "";
	while(*p) {
		if(*p != '%') {
			const char *q = strchr(p+1,'%');
			int n = !q ? ((int)strlen(p)) : ((int)(q - p));
			int avail = (int)(str_m-str_l);
			if(avail > 0) {
				register int k; register char *r; register const char* p1;
				for (p1=p, r=str+str_l, k=(n>avail?avail:n); k>0; k--)
					*r++ = *p1++;
			}
			p += n; str_l += n;
		} else {
			int min_field_width = 0, precision = 0;
			int zero_padding = 0, precision_specified = 0, justify_left = 0;
			int alternative_form = 0, force_sign = 0;
			int space_for_positive = 1;
			char data_type_modifier = '\0';
			char tmp[32];

			const char *str_arg = 0;
			int str_arg_l;

			long int long_arg;
			void *ptr_arg;
			int int_arg;
#ifdef SNPRINTF_LONGLONG_SUPPORT
			long long int long_long_arg = 0;
#endif
			int number_of_zeros_to_pad = 0;
			int zero_padding_insertion_ind = 0;
			char fmt_spec = '\0';

			p++;
			/* parse flags */
			while(*p == '0' || *p == '-' || *p == '+' ||
					*p == ' ' || *p == '#' || *p == '\'') {
				switch (*p) {
					case '0': zero_padding = 1; break;
					case '-': justify_left = 1; break;
					case '+': force_sign = 1; space_for_positive = 0; break;
					case ' ': force_sign = 1;
#ifdef PERL_COMPATIBLE
						  space_for_positive = 1;
#endif
						  break;
					case '#': alternative_form = 1; break;
					case '\'': break;
				}
				p++;
			}
			if(*p == '*') {
				p++; min_field_width = va_arg(ap, int);
				if(min_field_width < 0)
				{ min_field_width = -min_field_width; justify_left = 1; }
			} else if(isdigit((int)(*p))) {
				min_field_width = *p++ - '0';
				while(isdigit((int)(*p)))
					min_field_width = 10*min_field_width + (*p++ - '0');
			}
			if(*p == '.') {
				p++; precision_specified = 1;
				if(*p == '*') {
					p++; precision = va_arg(ap, int);
					if(precision < 0) {
						precision_specified = 0; precision = 0;
					}
				} else if(isdigit((int)(*p))) {
					precision = *p++ - '0';
					while(isdigit((int)(*p))) precision = 10*precision +
						(*p++ - '0');
				}
			}
			if(*p == 'h' || *p == 'l') {
				data_type_modifier = *p; p++;
				if(data_type_modifier == 'l' && *p == 'l') {
#ifdef SNPRINTF_LONGLONG_SUPPORT
					data_type_modifier = '2';
#else
					data_type_modifier = 'l';
#endif
					p++;
				}
			}
			fmt_spec = *p;
			switch (fmt_spec) {
				case 'i': fmt_spec = 'd'; break;
				case 'D': fmt_spec = 'd'; data_type_modifier = 'l'; break;
				case 'U': fmt_spec = 'u'; data_type_modifier = 'l'; break;
				case 'O': fmt_spec = 'o'; data_type_modifier = 'l'; break;
				default: break;
			}
			switch (fmt_spec) {
				case '%':
				case 'c':
				case 's':
					data_type_modifier = '\0';
#ifdef DIGITAL_UNIX_COMPATIBLE
					zero_padding = 0;
#endif
					str_arg_l = 1;
					switch (fmt_spec) {
						case '%':
							str_arg = p;
							break;
						case 'c':
							{ int j = va_arg(ap, int);
								str_arg = (const char*) &j; }
							break;
						case 's':
							str_arg = va_arg(ap, const char *);
							if(!str_arg) str_arg_l = 0;
							else if(!precision_specified)
								str_arg_l = strlen(str_arg);
							else if(precision <= 0) str_arg_l = 0;
							else {
								const char *q = memchr(str_arg,'\0',
										(size_t)precision);
								str_arg_l = !q ? precision :
									(q-str_arg);
							}
							break;
						default: break;
					}
					break;
				case 'd': case 'o': case 'u': case 'x': case 'X': case 'p':
					long_arg = 0; int_arg = 0; ptr_arg = NULL;
					if(fmt_spec == 'p') {
#ifdef SOLARIS_COMPATIBLE
#ifdef SOLARIS_BUG_COMPATIBLE
#else
						if(data_type_modifier == '2')
							data_type_modifier = '\0';
#endif
#else
						data_type_modifier = '\0';
#endif
						ptr_arg = va_arg(ap, void *);
						long_arg = !ptr_arg ? 0 : 1;
					} else {
						switch (data_type_modifier) {
							case '\0':
							case 'h':
								int_arg = va_arg(ap, int);
								long_arg = int_arg;
								break;
							case 'l':
								long_arg = va_arg(ap, long int);
								break;
#ifdef SNPRINTF_LONGLONG_SUPPORT
							case '2':
								long_long_arg = va_arg(ap,
										long long int);
								if(long_long_arg > 0)
									long_arg = +1;
								else if(long_long_arg < 0)
									long_arg = -1;
								else long_arg = 0;
								break;
#endif
						}
					}
					str_arg = tmp; str_arg_l = 0;
#ifndef PERL_COMPATIBLE
					if(precision_specified) zero_padding = 0;
#endif
					if(fmt_spec == 'd') {
						if(force_sign && long_arg >= 0)
							tmp[str_arg_l++] = space_for_positive ?
								' ' : '+';
					} else if(alternative_form) {
						if(long_arg != 0 && (fmt_spec == 'x' ||
									fmt_spec == 'X') )
						{ tmp[str_arg_l++] = '0'; tmp[str_arg_l++] =
							fmt_spec; }
#ifdef HPUX_COMPATIBLE
						else if(fmt_spec == 'p'
#ifndef HPUX_BUG_COMPATIBLE
								&& long_arg != 0
#endif
							) { tmp[str_arg_l++] = '0';
							tmp[str_arg_l++] = 'x'; }
#endif
					}
					zero_padding_insertion_ind = str_arg_l;
					if(!precision_specified) precision = 1;
					if(precision == 0 && long_arg == 0
#ifdef HPUX_BUG_COMPATIBLE
							&& fmt_spec != 'p'
#endif
					   ) {}
					else {
						char f[5]; int f_l = 0;
						f[f_l++] = '%';
						if(!data_type_modifier) { }
						else if(data_type_modifier=='2') {
							f[f_l++] = 'l';
							f[f_l++] = 'l';
						}
						else f[f_l++] = data_type_modifier;
						f[f_l++] = fmt_spec; f[f_l++] = '\0';
						if(fmt_spec == 'p') str_arg_l+=
							sprintf(tmp+
									str_arg_l, f, ptr_arg);
						else {
							switch (data_type_modifier) {
								case '\0':
								case 'h':
									str_arg_l+=
										sprintf(
												tmp+
												str_arg_l,
												f, int_arg);
									break;
								case 'l':
									str_arg_l+=sprintf(tmp+
											str_arg_l,
											f, long_arg);
									break;
#ifdef SNPRINTF_LONGLONG_SUPPORT
								case '2': str_arg_l+=sprintf(tmp+str_arg_l,f,long_long_arg); break;
#endif
							}
						}
						if(zero_padding_insertion_ind < str_arg_l &&
								tmp[zero_padding_insertion_ind] ==
								'-')
							zero_padding_insertion_ind++;
					}
					{ int num_of_digits = str_arg_l - zero_padding_insertion_ind;
						if(alternative_form && fmt_spec == 'o'
#ifdef HPUX_COMPATIBLE
								&& (str_arg_l > 0)
#endif
#ifdef DIGITAL_UNIX_BUG_COMPATIBLE
#else
								&& !(zero_padding_insertion_ind <
									str_arg_l
									&& tmp[
									zero_padding_insertion_ind]
									== '0')
#endif
						   ) {
							if(!precision_specified ||
									precision < num_of_digits+1)
							{ precision = num_of_digits+1;
								precision_specified = 1; }
						}
						/* zero padding to specified precision? */
						if(num_of_digits < precision)
							number_of_zeros_to_pad =
								precision - num_of_digits;
					}
					/* zero padding to specified minimal field width? */
					if(!justify_left && zero_padding) {
						int n = min_field_width -
							(str_arg_l+number_of_zeros_to_pad);
						if(n > 0) number_of_zeros_to_pad += n;
					}
					break;
				default:
					zero_padding = 0;
#ifndef DIGITAL_UNIX_COMPATIBLE
					justify_left = 1; min_field_width = 0;
#endif
#ifdef PERL_COMPATIBLE
					str_arg = starting_p; str_arg_l = p - starting_p;
#else
					str_arg = p; str_arg_l = 0;
#endif
					if(*p) str_arg_l++;
					break;
			}
			if(*p) p++;
			if(!justify_left) {
				int n = min_field_width - (str_arg_l+number_of_zeros_to_pad);
				if(n > 0) {
					int avail = (int)(str_m-str_l);
					if(avail > 0) {
						const char zp = (zero_padding ? '0' : ' ');
						register int k; register char *r;
						for (r=str+str_l, k=(n>avail?avail:n); k>0; k--)
							*r++ = zp;
					}
					str_l += n;
				}
			}
			if(number_of_zeros_to_pad <= 0) {
				zero_padding_insertion_ind = 0;
			} else {
				int n = zero_padding_insertion_ind;
				if(n > 0) {
					int avail = (int)(str_m-str_l);
					if(avail > 0)
						memcpy(str+str_l, str_arg,
								(size_t)(n>avail?avail:n));
					str_l += n;
				}
				n = number_of_zeros_to_pad;
				if(n > 0) {
					int avail = (int)(str_m-str_l);
					if(avail > 0) {
						register int k; register char *r;
						for (r=str+str_l, k=(n>avail?avail:n); k>0; k--)
							*r++ = '0';
					}
					str_l += n;
				}
			}

			{ int n = str_arg_l - zero_padding_insertion_ind;
				if(n > 0) {
					int avail = (int)(str_m-str_l);
					if(avail > 0)
						memcpy(str+str_l, str_arg+
								zero_padding_insertion_ind,
								(size_t)(n>avail ? avail : n) );
					str_l += n;
				}
			}

			if(justify_left) {
				int n = min_field_width - (str_arg_l+number_of_zeros_to_pad);
				if(n > 0) {
					int avail = (int)(str_m-str_l);
					if(avail > 0) {
						register int k; register char *r;
						for (r=str+str_l, k=(n>avail?avail:n); k>0; k--)
							*r++ = ' ';
					}
					str_l += n;
				}
			}
		}
	}
	va_end(ap);
	
	if(str_m > 0)
		str[str_l <= str_m-1 ? str_l : str_m-1] = '\0';

	return str_l;
}
#endif


/* Returns length of next utf-8 sequence */
int u8_seqlen(char *s)
{
	return trailingBytesForUTF8[(uint32_t)(unsigned char)s[0]] + 1;
}

/*
 * conversions without error checking
 * only works for valid UTF-8, i.e. no 5- or 6-byte sequences
 * srcsz = source size in bytes, or -1 if0-terminated
 * sz = dest size in # of wide characters
 *
 * returns # characters converted
 * dest will always be L'\0'-terminated, even ifthere isn't enough room
 * for all the characters
 * ifsz = srcsz+1 (i.e. 4*srcsz+4 bytes), there will always be enough space.
 */
int u8_toucs(uint32_t *dest, int sz, char *src, int srcsz)
{
	uint32_t ch;
	char *src_end = src + srcsz;
	int nb;
	int i = 0;

	while(i < sz - 1) {
		nb = trailingBytesForUTF8[(unsigned char)*src];
		if(srcsz == -1) {
			if(*src == 0)
				goto done_toucs;
		}
		else {
			if(src + nb >= src_end)
				goto done_toucs;
		}
		ch = 0;

		if(nb == 3) {ch += (unsigned char)*src++; ch <<= 6;}
		if(nb == 2) {ch += (unsigned char)*src++; ch <<= 6;}
		if(nb == 1) {ch += (unsigned char)*src++; ch <<= 6;}
		if(nb == 0) {ch += (unsigned char)*src++;}

		ch -= offsetsFromUTF8[nb];
		dest[i++] = ch;
	}

done_toucs:
	dest[i] = 0;
	return i;
}

/*
 * srcsz = number of source characters, or -1 if0-terminated
 * sz = size of dest buffer in bytes
 *
 * returns # characters converted
 * dest will only be '\0'-terminated ifthere is enough space. this is
 * for consistency; imagine there are 2 bytes of space left, but the next
 * character requires 3 bytes. in this case we could NUL-terminate, but in
 * general we can't when there's insufficient space. therefore this function
 * only NUL-terminates ifall the characters fit, and there's space for
 * the NUL as well.
 * the destination string will never be bigger than the source string.
 */
int u8_toutf8(char *dest, int sz, uint32_t *src, int srcsz)
{
	uint32_t ch;
	int i = 0;
	char *dest_end = dest + sz;

	while(srcsz < 0 ? src[i] != 0 : i < srcsz) {
		ch = src[i];
		if(ch < 0x80) {
			if(dest >= dest_end)
				return i;

			*dest++ = (char)ch;
		}
		else if(ch < 0x800) {
			if(dest >= dest_end-1)
				return i;

			*dest++ = (ch>>6) | 0xC0;
			*dest++ = (ch & 0x3F) | 0x80;
		}
		else if(ch < 0x10000) {
			if(dest >= dest_end-2)
				return i;

			*dest++ = (ch>>12) | 0xE0;
			*dest++ = ((ch>>6) & 0x3F) | 0x80;
			*dest++ = (ch & 0x3F) | 0x80;
		}
		else if(ch < 0x110000) {
			if(dest >= dest_end-3)
				return i;

			*dest++ = (ch>>18) | 0xF0;
			*dest++ = ((ch>>12) & 0x3F) | 0x80;
			*dest++ = ((ch>>6) & 0x3F) | 0x80;
			*dest++ = (ch & 0x3F) | 0x80;
		}
		i++;
	}

	if(dest < dest_end)
		*dest = '\0';

	return i;
}

int u8_wc_toutf8(char *dest, uint32_t ch)
{
	if(ch < 0x80) {
		dest[0] = (char)ch;
		return 1;
	}
	if(ch < 0x800) {
		dest[0] = (ch>>6) | 0xC0;
		dest[1] = (ch & 0x3F) | 0x80;
		return 2;
	}
	if(ch < 0x10000) {
		dest[0] = (ch>>12) | 0xE0;
		dest[1] = ((ch>>6) & 0x3F) | 0x80;
		dest[2] = (ch & 0x3F) | 0x80;
		return 3;
	}
	if(ch < 0x110000) {
		dest[0] = (ch>>18) | 0xF0;
		dest[1] = ((ch>>12) & 0x3F) | 0x80;
		dest[2] = ((ch>>6) & 0x3F) | 0x80;
		dest[3] = (ch & 0x3F) | 0x80;
		return 4;
	}

	return 0;
}

/* charnum => byte offset */
int u8_offset(char *str, int charnum)
{
	int offs = 0;

	while(charnum > 0 && str[offs]) {
		(void)(isutf(str[++offs]) || isutf(str[++offs]) ||
				isutf(str[++offs]) || ++offs);
		charnum--;
	}

	return offs;
}

/* byte offset => charnum */
int u8_charnum(char *s, int offset)
{
	int charnum = 0, offs = 0;

	while(offs < offset && s[offs]) {
		(void)(isutf(s[++offs]) || isutf(s[++offs]) ||
				isutf(s[++offs]) || ++offs);
		charnum++;
	}

	return charnum;
}

/* number of characters */
int u8_strlen(char *s)
{
	int count = 0;
	int i = 0;

	if(s == NULL || strlen(s) == 0)
		return 0;

	while(u8_nextchar(s, &i) != 0)
		count++;

	return count;
}

/* reads the next utf-8 sequence out of a string, updating an index */
uint32_t u8_nextchar(char *s, int *i)
{
	uint32_t ch = 0;
	int sz = 0;

	do {
		ch <<= 6;
		ch += (unsigned char)s[(*i)++];
		sz++;
	} while(s[*i] && !isutf(s[*i]));
	
	ch -= offsetsFromUTF8[sz-1];
	return ch;
}

void u8_inc(char *s, int *i)
{
	(void)(isutf(s[++(*i)]) || isutf(s[++(*i)]) ||
			isutf(s[++(*i)]) || ++(*i));
}

void u8_dec(char *s, int *i)
{
	(void)(isutf(s[--(*i)]) || isutf(s[--(*i)]) || 
			isutf(s[--(*i)]) || --(*i));
}

/* assumes that src points to the character after a backslash
   returns number of input characters processed */
int u8_read_escape_sequence(char *str, uint32_t *dest)
{
	uint32_t ch;
	char digs[9] = "\0\0\0\0\0\0\0\0";
	int dno = 0, i = 1;

	ch = (uint32_t)str[0];    /* take literal character */
	if(str[0] == 'n')
		ch = L'\n';
	else if(str[0] == 't')
		ch = L'\t';
	else if(str[0] == 'r')
		ch = L'\r';
	else if(str[0] == 'b')
		ch = L'\b';
	else if(str[0] == 'f')
		ch = L'\f';
	else if(str[0] == 'v')
		ch = L'\v';
	else if(str[0] == 'a')
		ch = L'\a';
	else if(octal_digit(str[0])) {
		i = 0;
		do {
			digs[dno++] = str[i++];
		} while(octal_digit(str[i]) && dno < 3);
		ch = strtol(digs, NULL, 8);
	}
	else if(str[0] == 'x') {
		while(hex_digit(str[i]) && dno < 2) {
			digs[dno++] = str[i++];
		}
		if(dno > 0)
			ch = strtol(digs, NULL, 16);
	}
	else if(str[0] == 'u') {
		while(hex_digit(str[i]) && dno < 4) {
			digs[dno++] = str[i++];
		}
		if(dno > 0)
			ch = strtol(digs, NULL, 16);
	}
	else if(str[0] == 'U') {
		while(hex_digit(str[i]) && dno < 8) {
			digs[dno++] = str[i++];
		}
		if(dno > 0)
			ch = strtol(digs, NULL, 16);
	}
	*dest = ch;

	return i;
}

/* convert a string with literal \uxxxx or \Uxxxxxxxx characters to UTF-8
example: u8_unescape(mybuf, 256, "hello\\u220e")
note the double backslash is needed ifcalled on a C string literal */
int u8_unescape(char *buf, int sz, char *src)
{
	int c=0, amt;
	uint32_t ch;
	char temp[4];

	while(*src && c < sz) {
		if(*src == '\\') {
			src++;
			amt = u8_read_escape_sequence(src, &ch);
		}
		else {
			ch = (uint32_t)*src;
			amt = 1;
		}
		src += amt;
		amt = u8_wc_toutf8(temp, ch);
		if(amt > sz-c)
			break;

		memcpy(&buf[c], temp, amt);
		c += amt;
	}
	if(c < sz)
		buf[c] = '\0';

	return c;
}

int u8_escape_wchar(char *buf, int sz, uint32_t ch)
{
	if(ch == L'\n')
		return u8_snprintf(buf, sz, "\\n");
	else if(ch == L'\t')
		return u8_snprintf(buf, sz, "\\t");
	else if(ch == L'\r')
		return u8_snprintf(buf, sz, "\\r");
	else if(ch == L'\b')
		return u8_snprintf(buf, sz, "\\b");
	else if(ch == L'\f')
		return u8_snprintf(buf, sz, "\\f");
	else if(ch == L'\v')
		return u8_snprintf(buf, sz, "\\v");
	else if(ch == L'\a')
		return u8_snprintf(buf, sz, "\\a");
	else if(ch == L'\\')
		return u8_snprintf(buf, sz, "\\\\");
	else if(ch < 32 || ch == 0x7f)
		return u8_snprintf(buf, sz, "\\x%hhX", (unsigned char)ch);
	else if(ch > 0xFFFF)
		return u8_snprintf(buf, sz, "\\U%.8X", (uint32_t)ch);
	else if(ch >= 0x80 && ch <= 0xFFFF)
		return u8_snprintf(buf, sz, "\\u%.4hX", (unsigned short)ch);

	return u8_snprintf(buf, sz, "%c", (char)ch);
}

int u8_escape(char *buf, int sz, char *src, int escape_quotes)
{
	int c = 0, i = 0, amt;

	while(src[i] && c < sz) {
		if(escape_quotes && src[i] == '"') {
			amt = u8_snprintf(buf, sz - c, "\\\"");
			i++;
		}
		else {
			amt = u8_escape_wchar(buf, sz - c,
					u8_nextchar(src, &i));
		}
		c += amt;
		buf += amt;
	}
	if(c < sz)
		*buf = '\0';

	return c;
}

char *u8_strchr(char *s, uint32_t ch, int *charn)
{
	int i = 0, lasti = 0;
	uint32_t c;

	*charn = 0;
	while(s[i]) {
		c = u8_nextchar(s, &i);
		if(c == ch)
			return &s[lasti];

		lasti = i;
		(*charn)++;
	}
	return NULL;
}

char *u8_memchr(char *s, uint32_t ch, size_t sz, int *charn)
{
	int i = 0, lasti = 0;
	uint32_t c;
	int csz;

	*charn = 0;
	while((size_t)i < sz) {
		c = csz = 0;
		do {
			c <<= 6;
			c += (unsigned char)s[i++];
			csz++;
		} while((size_t)i < sz && !isutf(s[i]));
		c -= offsetsFromUTF8[csz-1];

		if(c == ch)
			return &s[lasti];

		lasti = i;
		(*charn)++;
	}
	return NULL;
}

int u8_is_locale_utf8(char *locale)
{
	/* this code based on libutf8 */
	const char* cp = locale;

	for (; *cp != '\0' && *cp != '@' && *cp != '+' && *cp != ','; cp++) {
		if(*cp == '.') {
			const char* encoding = ++cp;
			for (; *cp != '\0' && *cp != '@' && *cp != '+' && *cp != ','; cp++)
				;
			if((cp-encoding == 5 &&
						!strncmp(encoding, "UTF-8", 5))
					|| (cp-encoding == 4 &&
						!strncmp(encoding, "utf8", 4)))
				return 1; /* it's UTF-8 */
			break;
		}
	}
	return 0;
}

int u8_strins(char *s, char *i, int p)
{
	int off = u8_offset(s, p);
	int len = strlen(s);
	int inlen = strlen(i);
	char *strbuf;

	if(!(strbuf = calloc(len + inlen + 1, sizeof(char))))
		return -1;

	memcpy(strbuf, s, off);
	memcpy(strbuf + off, i, inlen);
	memcpy(strbuf + off + inlen, s + off, len - off);

	memcpy(s, strbuf, len + inlen + 1);
	s[len + inlen + 1] = '\0';

	free(strbuf);

	return 0;
}

int u8_strdel(char *s, int p, int l)
{
	int len = strlen(s);
	int off = u8_offset(s, p);
	int endoff = u8_offset(s, p + l);

	memmove(s + off, s + endoff, len - endoff);

	s[len - (endoff - off)] = '\0';

	return 0;
}
