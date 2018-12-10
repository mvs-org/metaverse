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

#include <string.h>

#include "sha512EL.h"
#include "vrf.h"
#include "ed25519_ref10.h"
#include "randombytes.h"

namespace sodium {

int crypto_vrf_ietfdraft03_keypair(unsigned char pk[crypto_vrf_ietfdraft03_PUBLICKEYBYTES],
			       unsigned char sk[crypto_vrf_ietfdraft03_SECRETKEYBYTES])
{
    unsigned char seed[32];

    randombytes_buf(seed, sizeof seed);
    crypto_vrf_ietfdraft03_keypair_from_seed(pk, sk, seed);
    memset(seed, 0, sizeof seed);

    return 0;
}

int crypto_vrf_ietfdraft03_keypair_from_seed(unsigned char pk[crypto_vrf_ietfdraft03_PUBLICKEYBYTES],
					 unsigned char sk[crypto_vrf_ietfdraft03_SECRETKEYBYTES],
					 const unsigned char seed[crypto_vrf_ietfdraft03_SEEDBYTES])
{
    ge25519_p3 A;

    crypto_hash_sha512(sk, seed, 32);
    sk[0] &= 248;
    sk[31] &= 127;
    sk[31] |= 64;
    ge25519_scalarmult_base(&A, sk);
    ge25519_p3_tobytes(pk, &A);
    memmove(sk, seed, 32);
    memmove(sk + 32, pk, 32);

    return 0;
}

void crypto_vrf_ietfdraft03_sk_to_pk(unsigned char pk[crypto_vrf_ietfdraft03_PUBLICKEYBYTES],
				const unsigned char skpk[crypto_vrf_ietfdraft03_SECRETKEYBYTES])
{
    memmove(pk, skpk+32, 32);
}

void crypto_vrf_ietfdraft03_sk_to_seed(unsigned char seed[crypto_vrf_ietfdraft03_SEEDBYTES],
				  const unsigned char skpk[crypto_vrf_ietfdraft03_SECRETKEYBYTES])
{
    memmove(seed, skpk, 32);
}
}