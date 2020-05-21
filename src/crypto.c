#include "crypto.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

extern void gen_keys(struct pvt_key *pvt, struct pub_key *pub)
{
	char buf[BUFFER_SIZE];
	int i;
	mpz_t phi;
	mpz_t tmp1;
	mpz_t tmp2;

	mpz_init(phi);
	mpz_init(tmp1);
	mpz_init(tmp2);

	srand(time(NULL));

	mpz_set_ui(pvt->e, 3); 

	for(i = 0; i < BUFFER_SIZE; i++)
		buf[i] = rand() % 0xFF;

	buf[0] |= 0xC0;

	buf[BUFFER_SIZE - 1] |= 0x01;

	mpz_import(tmp1, BUFFER_SIZE, 1, sizeof(char), 0, 0, buf);

	mpz_nextprime(pvt->p, tmp1);

	mpz_mod(tmp2, pvt->p, pvt->e);
	while(!mpz_cmp_ui(tmp2, 1)) {
		mpz_nextprime(pvt->p, pvt->p);
		mpz_mod(tmp2, pvt->p, pvt->e);
	}

	do {
		for(i = 0; i < BUFFER_SIZE; i++)
			buf[i] = rand() % 0xFF;

		buf[0] |= 0xC0;
		buf[BUFFER_SIZE - 1] |= 0x01;
		mpz_import(tmp1, BUFFER_SIZE, 1, sizeof(char), 0, 0, buf);
		mpz_nextprime(pvt->q, tmp1);
		mpz_mod(tmp2, pvt->q, pvt->e);
		while(!mpz_cmp_ui(tmp2, 1)) {
			mpz_nextprime(pvt->q, pvt->q);
			mpz_mod(tmp2, pvt->q, pvt->e);
		}
	} while(mpz_cmp(pvt->p, pvt->q) == 0);

	mpz_mul(pvt->n, pvt->p, pvt->q);

	mpz_sub_ui(tmp1, pvt->p, 1);
	mpz_sub_ui(tmp2, pvt->q, 1);
	mpz_mul(phi, tmp1, tmp2);

	if(mpz_invert(pvt->d, pvt->e, phi) == 0) {
		mpz_gcd(tmp1, pvt->e, phi);
		printf("gcd(e, phi) = [%s]\n", mpz_get_str(NULL, 16, tmp1));
		printf("Invert failed\n");
	}

	mpz_set(pub->e, pvt->e);
	mpz_set(pub->n, pvt->n);

	mpz_clear(phi);
	mpz_clear(tmp1);
	mpz_clear(tmp2);
}


extern void free_keys(struct pvt_key *pvt, struct pub_key *pub)
{
	if(pvt != NULL) {
		mpz_clear(pvt->n);
		mpz_clear(pvt->e);
		mpz_clear(pvt->d);
		mpz_clear(pvt->p);
		mpz_clear(pvt->q);
	}

	if(pub != NULL) {
		mpz_clear(pub->n);
		mpz_clear(pub->e);
	}
}


extern int encrypt(char **out, int *out_len, char *in, int in_len, 
		struct pub_key pub)
{
	char *ret;
	char block[BLOCK_SIZE];
	mpz_t m;
	mpz_t c;

	int i = 0;
	int left = in_len;
	int num = (int)ceil((double)in_len / BUF_SIZE);
	int size = num * BLOCK_SIZE;

	if(!(ret = malloc(size)))
		return -1;

	memset(ret, 0, size);

	mpz_inits(m, c, NULL);

	while(left > 0) {
		int from = in_len - left;
		int to = (i + 1) * BLOCK_SIZE;
		int sz = (left > BUF_SIZE) ? (BUF_SIZE) : (left);
		size_t enc_len = 0;	

		memset(block, 0, BLOCK_SIZE);
		block[0] = 0x01;
		block[1] = 0x02;
		memcpy(block + 2, in + from, sz);

		mpz_import(m, BLOCK_SIZE, 1, sizeof(block[0]), 
				0, 0, block);

		mpz_powm(c, m, pub.e, pub.n);

		memset(block, 0, BLOCK_SIZE);

		mpz_export(block, &enc_len, 1, sizeof(char), 0, 0, c);

		memcpy(ret + to - enc_len, block, enc_len);

		left -= sz;
		i++;
	}

	*out = ret;
	*out_len = size;
	mpz_clears(m, c, NULL);
	return 0;
} 


extern int decrypt(char **out, int *out_len, char *in, int in_len, 
		struct pvt_key pvt)
{
	int i;
	int num = in_len / BLOCK_SIZE;
	int msg_idx = 0;
	char block[BLOCK_SIZE];
	int size = num * BUF_SIZE;
	char *ret;
	mpz_t m;
	mpz_t c;

	if(!(ret = malloc(size)))
		return -1;

	memset(ret, 0, size);

	mpz_inits(m, c, NULL);

	for(i = 0; i < num; i++) {
		mpz_import(c, BLOCK_SIZE, 1, sizeof(char), 0, 0, 
				in + (i * BLOCK_SIZE));

		mpz_powm(m, c, pvt.d, pvt.n);

		mpz_export(block, NULL, 1, sizeof(char), 0, 0, m);

		memcpy(ret + (i * BUF_SIZE), block + 2, BUF_SIZE);
	}

	*out = ret;
	*out_len = size;
	mpz_clears(m, c, NULL);
	return msg_idx;
}
