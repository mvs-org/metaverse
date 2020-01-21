/*
Copyright (c) 2019 - 2020 Amir Hossein Alikhah Mishamandani
Copyright (c) 2020 Algorand LLC

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

#ifndef SHA512EL_H
#define SHA512EL_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

namespace sodium {

typedef struct crypto_hash_sha512_state {
    uint64_t state[8];
    uint64_t count[2];
    uint8_t  buf[128];
} crypto_hash_sha512_state;


size_t crypto_hash_sha512_statebytes(void);

#define crypto_hash_sha512_BYTES 64U
size_t crypto_hash_sha512_bytes(void);


int crypto_hash_sha512(unsigned char *out, const unsigned char *in,
                       unsigned long long inlen);


int crypto_hash_sha512_init(crypto_hash_sha512_state *state);


int crypto_hash_sha512_update(crypto_hash_sha512_state *state,
                              const unsigned char *in,
                              unsigned long long inlen);


int crypto_hash_sha512_final(crypto_hash_sha512_state *state,
                             unsigned char *out);

}

#endif
