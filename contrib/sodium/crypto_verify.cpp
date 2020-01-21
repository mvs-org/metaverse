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
#include <stddef.h>
#include <stdint.h>

#include "crypto_verify_16.h"

namespace sodium {

size_t crypto_verify_16_bytes(void){
	
    return crypto_verify_16_BYTES;
}


static inline int
crypto_verify_n(const unsigned char *x_, const unsigned char *y_,
                const int n)
{
    const volatile unsigned char *volatile x =
        (const volatile unsigned char *volatile) x_;
    const volatile unsigned char *volatile y =
        (const volatile unsigned char *volatile) y_;
    volatile uint_fast16_t d = 0U;
    int i;

    for (i = 0; i < n; i++) {
        d |= x[i] ^ y[i];
    }
    return (1 & ((d - 1) >> 8)) - 1;
}

int crypto_verify_16(const unsigned char *x, const unsigned char *y)
{
    return crypto_verify_n(x, y, crypto_verify_16_BYTES);
}


}