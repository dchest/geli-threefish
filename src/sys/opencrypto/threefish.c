/*-
* Copyright (c) 2012 Dmitry Chestnykh <dmitry@codingrobots.com>
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS'' AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
* OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
* SUCH DAMAGE.
*/

#include "threefish.h"

#if defined(__i386__) || defined(__x86_64__)

# define LOAD64_LE(p)		(*(uint64_t *)(p))
# define STORE64_LE(p, v)	do { (*(uint64_t *)(p)) = (v); } while(0)

#else

# define LOAD64_LE(p)							\
	 (((uint64_t)((p)[0]))     | ((uint64_t)((p)[1])<< 8) |		\
	  ((uint64_t)((p)[2])<<16) | ((uint64_t)((p)[3])<<24) |		\
	  ((uint64_t)((p)[4])<<32) | ((uint64_t)((p)[5])<<40) |		\
	  ((uint64_t)((p)[6])<<48) | ((uint64_t)((p)[7])<<56))

# define STORE64_LE(p, v) do {						\
	(p)[0] = (uint8_t)(v);						\
	(p)[1] = (uint8_t)((v)>> 8);					\
	(p)[2] = (uint8_t)((v)>>16);					\
	(p)[3] = (uint8_t)((v)>>24);					\
	(p)[4] = (uint8_t)((v)>>32);					\
	(p)[5] = (uint8_t)((v)>>40);					\
	(p)[6] = (uint8_t)((v)>>48);					\
	(p)[7] = (uint8_t)((v)>>56);					\
} while (0)

#endif

#define KEYSCHEDULE_PARITY 0x1BD11BDAA9FC1A22ULL

static const int RotK[8][4] = {
	{ 46, 36, 19, 37 },
	{ 33, 27, 14, 42 },
	{ 17, 49, 36, 39 },
	{ 44,  9, 54, 56 },
	{ 39, 30, 34, 24 },
	{ 13, 50, 10, 17 },
	{ 25, 29, 39, 43 },
	{  8, 35, 56, 22 }
};

void
threefish_expand_key(uint64_t ks[9], const uint8_t k[64])
{

	ks[0] = LOAD64_LE(&k[ 0]);
	ks[1] = LOAD64_LE(&k[ 8]);
	ks[2] = LOAD64_LE(&k[16]);
	ks[3] = LOAD64_LE(&k[24]);
	ks[4] = LOAD64_LE(&k[32]);
	ks[5] = LOAD64_LE(&k[40]);
	ks[6] = LOAD64_LE(&k[48]);
	ks[7] = LOAD64_LE(&k[56]);
	ks[8] = KEYSCHEDULE_PARITY ^ ks[0] ^ ks[1] ^ ks[2] ^ ks[3] ^ ks[4] ^
			ks[5] ^ ks[6] ^ ks[7];
}

#define ROTL64(x, n) (((x) << (n)) | ((x) >> (64 - (n))))
#define ROTR64(x, n) (((x) >> (n)) | ((x) << (64 - (n))))

/* Inlining this function makes it 3x slower. */
void __attribute__((noinline))
threefish_encrypt_block(const uint64_t ks[9], const uint64_t ts[3],
		const uint8_t in[64], uint8_t out[64])
{
	uint64_t x0 = LOAD64_LE(&in[ 0]);
	uint64_t x1 = LOAD64_LE(&in[ 8]);
	uint64_t x2 = LOAD64_LE(&in[16]);
	uint64_t x3 = LOAD64_LE(&in[24]);
	uint64_t x4 = LOAD64_LE(&in[32]);
	uint64_t x5 = LOAD64_LE(&in[40]);
	uint64_t x6 = LOAD64_LE(&in[48]);
	uint64_t x7 = LOAD64_LE(&in[56]);
	
#define MIX(a, b, rotk) ((a) += (b), (b) = ROTL64((b), (rotk)) ^ (a))

#define ER(n0, n1, n2, n3, n4, n5, n6, n7, r) do {			\
	MIX(x##n0, x##n1, RotK[r][0]);					\
	MIX(x##n2, x##n3, RotK[r][1]);					\
	MIX(x##n4, x##n5, RotK[r][2]);					\
	MIX(x##n6, x##n7, RotK[r][3]);					\
} while (0)

#define EI(r) do {							\
	x0 += ks[((r)+1) % 9];						\
	x1 += ks[((r)+2) % 9];						\
	x2 += ks[((r)+3) % 9];						\
	x3 += ks[((r)+4) % 9];						\
	x4 += ks[((r)+5) % 9];						\
	x5 += ks[((r)+6) % 9] + ts[((r)+1) % 3];			\
	x6 += ks[((r)+7) % 9] + ts[((r)+2) % 3];			\
	x7 += ks[((r)+8) % 9] + (r)+1;					\
} while (0)

#define EROUNDS(r) do {							\
	ER(0, 1, 2, 3, 4, 5, 6, 7, 0);					\
	ER(2, 1, 4, 7, 6, 5, 0, 3, 1);					\
	ER(4, 1, 6, 3, 0, 5, 2, 7, 2);					\
	ER(6, 1, 0, 7, 2, 5, 4, 3, 3);					\
	EI(2*(r));							\
	ER(0, 1, 2, 3, 4, 5, 6, 7, 4);					\
	ER(2, 1, 4, 7, 6, 5, 0, 3, 5);					\
	ER(4, 1, 6, 3, 0, 5, 2, 7, 6);					\
	ER(6, 1, 0, 7, 2, 5, 4, 3, 7);					\
	EI(2*(r)+1);							\
} while (0)

	EI(-1);
	EROUNDS(0); EROUNDS(1); EROUNDS(2);
	EROUNDS(3); EROUNDS(4); EROUNDS(5);
	EROUNDS(6); EROUNDS(7); EROUNDS(8);
 
#undef EROUNDS
#undef EI
#undef ER

	STORE64_LE(&out[ 0], x0);
	STORE64_LE(&out[ 8], x1);
	STORE64_LE(&out[16], x2);
	STORE64_LE(&out[24], x3);
	STORE64_LE(&out[32], x4);
	STORE64_LE(&out[40], x5);
	STORE64_LE(&out[48], x6);
	STORE64_LE(&out[56], x7);
}

/* Inlining this function makes it 3x slower. */
void __attribute__((noinline))
threefish_decrypt_block(const uint64_t ks[9], const uint64_t ts[3],
		const uint8_t in[64], uint8_t out[64])
{
	uint64_t x0 = LOAD64_LE(&in[ 0]);
	uint64_t x1 = LOAD64_LE(&in[ 8]);
	uint64_t x2 = LOAD64_LE(&in[16]);
	uint64_t x3 = LOAD64_LE(&in[24]);
	uint64_t x4 = LOAD64_LE(&in[32]);
	uint64_t x5 = LOAD64_LE(&in[40]);
	uint64_t x6 = LOAD64_LE(&in[48]);
	uint64_t x7 = LOAD64_LE(&in[56]);
	
#define UNMIX(a, b, rotk) ((b) = ROTR64((b) ^ ((a)), (rotk)), (a) -= (b))

#define DR(n0, n1, n2, n3, n4, n5, n6, n7, r) do {			\
	UNMIX(x##n0, x##n1, RotK[r][0]);				\
	UNMIX(x##n2, x##n3, RotK[r][1]);				\
	UNMIX(x##n4, x##n5, RotK[r][2]);				\
	UNMIX(x##n6, x##n7, RotK[r][3]);				\
} while (0)

#define DI(R) do {							\
	x0 -= ks[((R)+1) % 9];						\
	x1 -= ks[((R)+2) % 9];						\
	x2 -= ks[((R)+3) % 9];						\
	x3 -= ks[((R)+4) % 9];						\
	x4 -= ks[((R)+5) % 9];						\
	x5 -= ks[((R)+6) % 9] + ts[((R)+1) % 3];			\
	x6 -= ks[((R)+7) % 9] + ts[((R)+2) % 3];			\
	x7 -= ks[((R)+8) % 9] + (R)+1;					\
} while (0)

#define DROUNDS(R) do {							\
	DI(2*(R)+1);							\
	DR(6, 1, 0, 7, 2, 5, 4, 3, 7);					\
	DR(4, 1, 6, 3, 0, 5, 2, 7, 6);					\
	DR(2, 1, 4, 7, 6, 5, 0, 3, 5);					\
	DR(0, 1, 2, 3, 4, 5, 6, 7, 4);					\
	DI(2*(R));							\
	DR(6, 1, 0, 7, 2, 5, 4, 3, 3);					\
	DR(4, 1, 6, 3, 0, 5, 2, 7, 2);					\
	DR(2, 1, 4, 7, 6, 5, 0, 3, 1);					\
	DR(0, 1, 2, 3, 4, 5, 6, 7, 0);					\
} while (0)

	DROUNDS(8); DROUNDS(7); DROUNDS(6);
	DROUNDS(5); DROUNDS(4); DROUNDS(3);
	DROUNDS(2); DROUNDS(1); DROUNDS(0);
	DI(-1);
 
#undef DROUNDS
#undef DI
#undef DR

	STORE64_LE(&out[ 0], x0);
	STORE64_LE(&out[ 8], x1);
	STORE64_LE(&out[16], x2);
	STORE64_LE(&out[24], x3);
	STORE64_LE(&out[32], x4);
	STORE64_LE(&out[40], x5);
	STORE64_LE(&out[48], x6);
	STORE64_LE(&out[56], x7);
}

/* Make sure the compiler won't optimize this away. */
static void __attribute__((noinline))
clear_memory(void *p, size_t len)
{
	size_t i;
	uint8_t *b = (uint8_t *)p;

	for (i = 0; i < len; i++)
		b[i] = 0;
}

void
threefish_encrypt(const uint8_t key[64], uint64_t sectornum, const uint8_t *in,
		size_t inlen, uint8_t *out)
{
	uint64_t ks[9], ts[3];

	expand_key(ks, key);

	ts[0] = sectornum;
	ts[1] = 0;	/* 2nd part of tweak is block counter */

	while (inlen > 0) {
		ts[2] = ts[0] ^ ts[1];
		encrypt_block(ks, ts, in, out);
		ts[1]++;	/* increment block counter */

		in += 64; out += 64; inlen -= 64;
	}
	clear_memory(ks, sizeof(ks));
}

void
threefish_decrypt(const uint8_t key[64], uint64_t sectornum, const uint8_t *in,
		size_t inlen, uint8_t *out)
{
	uint64_t ks[9], ts[3];

	expand_key(ks, key);

	ts[0] = sectornum;
	ts[1] = 0;	/* 2nd part of tweak is block counter */

	while (inlen > 0) {
		ts[2] = ts[0] ^ ts[1];
		decrypt_block(ks, ts, in, out);
		ts[1]++;	/* increment block counter */

		in += 64; out += 64; inlen -= 64;
	}
	clear_memory(ks, sizeof(ks));
}



#if 0

/* Test */
#include <stdio.h>
#include <time.h>

int
main(void)
{
	uint8_t key[64];
	uint8_t tweak[16] = {1};
	uint8_t in[4096];
	uint8_t out[4096];
	int i;
	double d;

	for (i = 0; i < sizeof(key); i++) {
		key[i] = i+1;
	}
	for (i = 0; i < sizeof(in); i++) {
		in[i] = i;
	}

	uint64_t bytes = 100*1024*1024;
	clock_t cl = clock();
	for (i = 0; i < bytes; i += sizeof(in)) {
		threefish_encrypt(key, 0, in, sizeof(in), out);
		in[0] = out[0];
	}
	d = (clock()-cl)/(double)CLOCKS_PER_SEC;

	printf("\n\n%.3f sec, %.3f MB/s\n\n", d, bytes/1024/1024/d);

	return 0;
}

#endif

