#ifndef THREEFISH_H_
#define THREEFISH_H_

#include <stddef.h>
#include <stdint.h>

/*
 * threefish_encrypt(key, tweak0, in, inlen, out)
 * 
 * Encrypt blocks from in to out using 64-byte key and 64-bit tweak.
 */
void threefish_encrypt(const uint8_t[64], const uint64_t, const uint8_t *,
		size_t, uint8_t *);

/*
 * threefish_encrypt(key, tweak, input, output)
 * 
 * Decrypt blocks from in to out using 64-byte key and 64-bit tweak.
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

#endif /* !THREEFISH_H_ */
