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

#ifndef _THREEFISH_H_
#define _THREEFISH_H_

#include <stddef.h>
#include <stdint.h>

/*
 * threefish_encrypt(key, sectornum, in, inlen, out)
 * 
 * Encrypt data of length inlen from in to out using 64-byte key
 * and 64-bit sector number. Data length must be a multiple of 64.
 *
 * Data is encrypted using the "tweak counter mode": for each 64-byte block
 * of data, the given 64-bit sector number is combined with 64-bit block
 * counter to get the full 128-bit tweak for Threefish, which is then used,
 * along with the key, for encryption.
 */
void threefish_encrypt(const uint8_t[64], const uint64_t, const uint8_t *,
		size_t, uint8_t *);

/*
 * threefish_encrypt(key, sectornum, in, inlen, out)
 * 
 * Decrypt data of length inlen from in to out using 64-byte key
 * and 64-bit sector number. Data length must be a multiple of 64.
 */
void threefish_decrypt(const uint8_t[64], const uint64_t, const uint8_t *,
		size_t, uint8_t *);

/*
 * Fishthree is Threefish with encryption and decryption swapped.
 *
 * Since encryption in Threefish is faster than decryption, the Skein paper
 * recommends swapping the two operations to achieve better performance
 * if decryption is more common that encryption.
 */
#define fishthree_encrypt threefish_decrypt
#define fishthree_decrypt threefish_encrypt

/*
 * Low-level operations.
 */

void threefish_expand_key(uint64_t[9], const uint8_t[64]);

void threefish_encrypt_block(const uint64_t[9], const uint64_t[3],
		const uint8_t[64], uint8_t[64]);

void threefish_decrypt_block(const uint64_t[9], const uint64_t[3],
		const uint8_t[64], uint8_t[64]);

#endif /* !_THREEFISH_H_ */
