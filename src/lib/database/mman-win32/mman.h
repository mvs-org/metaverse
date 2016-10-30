
/* mman-win32 from code.google.com/p/mman-win32 (MIT License). */

#ifndef LIBBITCOIN_DATABASE_MMAN_H
#define LIBBITCOIN_DATABASE_MMAN_H

#ifdef _WIN32

/* DO NOT USE off_t/_off_t AS THE SIZE VARIES. */
#include <stddef.h>
typedef size_t oft__;

/* mman-win32 from code.google.com/p/mman-win32 (MIT License). */
#ifdef __cplusplus
extern "C" {
#endif

#define PROT_NONE       0
#define PROT_READ       1
#define PROT_WRITE      2
#define PROT_EXEC       4

#define MAP_FILE        0
#define MAP_SHARED      1
#define MAP_PRIVATE     2
#define MAP_TYPE        0xf
#define MAP_FIXED       0x10
#define MAP_ANONYMOUS   0x20
#define MAP_ANON        MAP_ANONYMOUS

#define MAP_FAILED      ((void*)-1)

/* Flags for msync. */
#define MS_ASYNC        1
#define MS_SYNC         2
#define MS_INVALIDATE   4

/* Flags for madvise (stub). */
#define MADV_RANDOM     0

void* mmap(void* addr, size_t len, int prot, int flags, int fildes, oft__ off);
int munmap(void* addr, size_t len);
int madvise(void* addr, size_t len, int advice);
int mprotect(void* addr, size_t len, int prot);
int msync(void* addr, size_t len, int flags);
int mlock(const void* addr, size_t len);
int munlock(const void* addr, size_t len);
int fsync(int fd);
int ftruncate(int fd, oft__ size);

#ifdef __cplusplus
};
#endif

#else
typedef int make_iso_compiler_not_complain;
#endif

#endif
