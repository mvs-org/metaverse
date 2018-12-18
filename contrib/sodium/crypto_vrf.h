/*
Copyright (c) 2018 Amir Hossein Alikhah Mishamandani
Copyright (c) 2018 Algorand LLC

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef crypto_vrf_H
#define crypto_vrf_H

/*
 * THREAD SAfe25519TY: crypto_vrf_keypair() is thread-safe25519 provided that
 * sodium_init() was called before.
 *
 * Other functions, including crypto_vrf_keypair_from_seed(), are always
 * thread-safe25519.
 */

#include <stddef.h>

#include "vrf.h"

#ifdef __cplusplus
extern "C" {
#endif

namespace sodium {

#define crypto_vrf_PUBLICKEYBYTES crypto_vrf_ietfdraft03_PUBLICKEYBYTES
size_t crypto_vrf_publickeybytes(void);

#define crypto_vrf_SECRETKEYBYTES crypto_vrf_ietfdraft03_SECRETKEYBYTES
size_t crypto_vrf_secretkeybytes(void);

#define crypto_vrf_SEEDBYTES crypto_vrf_ietfdraft03_SEEDBYTES
size_t crypto_vrf_seedbytes(void);

#define crypto_vrf_PROOFBYTES crypto_vrf_ietfdraft03_PROOFBYTES
size_t crypto_vrf_proofbytes(void);

#define crypto_vrf_OUTPUTBYTES crypto_vrf_ietfdraft03_OUTPUTBYTES
size_t crypto_vrf_outputbytes(void);

#define crypto_vrf_PRIMITIVE "ietfdraft03"
const char *crypto_vrf_primitive(void);

int crypto_vrf_keypair(unsigned char *pk, unsigned char *sk);

int crypto_vrf_keypair_from_seed(unsigned char *pk, unsigned char *sk,
				 const unsigned char *seed);

int crypto_vrf_is_valid_key(const unsigned char *pk)
            __attribute__ ((warn_unused_result));

int crypto_vrf_prove(unsigned char *proof, const unsigned char *sk,
		     const unsigned char *m, unsigned long long mlen);

int crypto_vrf_verify(unsigned char *output,
		      const unsigned char *pk,
		      const unsigned char *proof,
		      const unsigned char *m, unsigned long long mlen)
            __attribute__ ((warn_unused_result));

int crypto_vrf_proof_to_hash(unsigned char *hash, const unsigned char *proof);

void crypto_vrf_sk_to_pk(unsigned char *pk, const unsigned char *skpk);

void crypto_vrf_sk_to_seed(unsigned char *seed, const unsigned char *skpk);

}

#ifdef __cplusplus
}
#endif

#endif
