/*
Copyright (c) 2020 Amir Hossein Alikhah Mishamandani
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
#ifndef randombytes_H
#define randombytes_H

#include <stddef.h>
#include <stdint.h>

#include <sys/types.h>

namespace sodium {

typedef struct randombytes_implementation {
    const char *(*implementation_name)(void); /* required */
    uint32_t    (*random)(void);              /* required */
    uint32_t    (*uniform)(const uint32_t upper_bound); /* optional, a default implementation will be used if NULL */
    void        (*buf)(void * const buf, const size_t size); /* required */
    int         (*close)(void);               /* optional */
} randombytes_implementation;

#define randombytes_SEEDBYTES 32U
size_t randombytes_seedbytes(void);

void randombytes_buf(void * const buf, const size_t size);

uint32_t randombytes_random(void);

uint32_t randombytes_uniform(const uint32_t upper_bound);

int randombytes_close(void);

int randombytes_set_implementation(randombytes_implementation *impl);

const char *randombytes_implementation_name(void);

}

#endif
