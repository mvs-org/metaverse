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

#include <stdio.h>
#include "vrf.h"
#include "crypto_vrf.h"

namespace sodium {

size_t
crypto_vrf_publickeybytes(void)
{
    return crypto_vrf_PUBLICKEYBYTES;
}

size_t
crypto_vrf_secretkeybytes(void)
{
    return crypto_vrf_SECRETKEYBYTES;
}

size_t
crypto_vrf_seedbytes(void)
{
    return crypto_vrf_SEEDBYTES;
}

size_t
crypto_vrf_proofbytes(void)
{
    return crypto_vrf_PROOFBYTES;
}

size_t
crypto_vrf_outputbytes(void)
{
    return crypto_vrf_OUTPUTBYTES;
}

const char *
crypto_vrf_primitive(void)
{
    return crypto_vrf_PRIMITIVE;
}

int
crypto_vrf_keypair(unsigned char *pk, unsigned char *sk)
{
    return crypto_vrf_ietfdraft03_keypair(pk, sk);
}

int crypto_vrf_keypair_from_seed(unsigned char *pk, unsigned char *sk, const unsigned char *seed){
	
    return crypto_vrf_ietfdraft03_keypair_from_seed(pk, sk, seed);
}

int
crypto_vrf_is_valid_key(const unsigned char *pk)
{
    return crypto_vrf_ietfdraft03_is_valid_key(pk);
}

int
crypto_vrf_prove(unsigned char *proof, const unsigned char *skpk,
		 const unsigned char *m, const unsigned long long mlen)
{
    return crypto_vrf_ietfdraft03_prove(proof, skpk, m, mlen);
}

int
crypto_vrf_verify(unsigned char *output, const unsigned char *pk,
		  const unsigned char *proof, const unsigned char *m,
		  const unsigned long long mlen)
{
    return crypto_vrf_ietfdraft03_verify(output, pk, proof, m, mlen);
}

int
crypto_vrf_proof_to_hash(unsigned char *hash, const unsigned char *proof)
{
    return crypto_vrf_ietfdraft03_proof_to_hash(hash, proof);
}

void
crypto_vrf_sk_to_pk(unsigned char *pk, const unsigned char *skpk)
{
    crypto_vrf_ietfdraft03_sk_to_pk(pk, skpk);
}

void
crypto_vrf_sk_to_seed(unsigned char *seed, const unsigned char *skpk)
{
    crypto_vrf_ietfdraft03_sk_to_seed(seed, skpk);
}
}