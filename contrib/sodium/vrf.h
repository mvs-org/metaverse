/*
Copyright (c) 2018 - 2019 Amir Hossein Alikhah Mishamandani

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
#include <stddef.h>
#ifndef VRF_H
#define VRF_H

namespace sodium {
	
#ifdef __cplusplus
extern "C" {
#endif


#define crypto_vrf_ietfdraft03_PUBLICKEYBYTES 32U
size_t crypto_vrf_ietfdraft03_publickeybytes(void);

#define crypto_vrf_ietfdraft03_SECRETKEYBYTES 64U
size_t crypto_vrf_ietfdraft03_secretkeybytes(void);

#define crypto_vrf_ietfdraft03_SEEDBYTES 32U
size_t crypto_vrf_ietfdraft03_seedbytes(void);

#define crypto_vrf_ietfdraft03_PROOFBYTES 80U
size_t crypto_vrf_ietfdraft03_proofbytes(void);

#define crypto_vrf_ietfdraft03_OUTPUTBYTES 64U
size_t crypto_vrf_ietfdraft03_outputbytes(void);

int crypto_vrf_ietfdraft03_prove(unsigned char *proof, 
					const unsigned char *sk,
					const unsigned char *m,
					unsigned long long mlen);

int crypto_vrf_ietfdraft03_keypair(unsigned char pk[crypto_vrf_ietfdraft03_PUBLICKEYBYTES],
			       unsigned char sk[crypto_vrf_ietfdraft03_SECRETKEYBYTES]);

int crypto_vrf_ietfdraft03_keypair_from_seed(unsigned char pk[crypto_vrf_ietfdraft03_PUBLICKEYBYTES],
					 unsigned char sk[crypto_vrf_ietfdraft03_SECRETKEYBYTES],
					 const unsigned char seed[crypto_vrf_ietfdraft03_SEEDBYTES]);

void crypto_vrf_ietfdraft03_sk_to_pk(unsigned char pk[crypto_vrf_ietfdraft03_PUBLICKEYBYTES],
				const unsigned char skpk[crypto_vrf_ietfdraft03_SECRETKEYBYTES]);

void crypto_vrf_ietfdraft03_sk_to_seed(unsigned char seed[crypto_vrf_ietfdraft03_SEEDBYTES],
				  const unsigned char skpk[crypto_vrf_ietfdraft03_SECRETKEYBYTES]);				

int crypto_vrf_ietfdraft03_is_valid_key(const unsigned char *pk)
            __attribute__ ((warn_unused_result));

int crypto_vrf_ietfdraft03_verify(unsigned char *output,
				  const unsigned char *pk,
				  const unsigned char *proof,
				  const unsigned char *m,
				  unsigned long long mlen)
            __attribute__ ((warn_unused_result));

int crypto_vrf_ietfdraft03_proof_to_hash(unsigned char *hash,
				         const unsigned char *proof);


#ifdef __cplusplus
}
#endif

}

#endif