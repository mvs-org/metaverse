/* mman-win32 derived from code.google.com/p/mman-win32 (MIT License). */

#include "mman.h"

#ifdef _WIN32

#include <stdint.h>
#include <windows.h>
#include <errno.h>
#include <io.h>

#ifndef FILE_MAP_EXECUTE
    #define FILE_MAP_EXECUTE 0x0020
#endif

/* local utilities */

static const int large = (sizeof(oft__) > sizeof(DWORD));

static int last_error(int default_value)
{
    /* TODO: implement full mapping to standard codes. */

    switch (GetLastError())
    {
        case ERROR_INVALID_HANDLE:
            return EBADF;
        case ERROR_DISK_FULL:
            return ENOSPC;
        default:
            return default_value;
    }
}

static DWORD protect_page(int prot)
{
    DWORD protect = 0;

    if (prot == PROT_NONE)
        return protect;

    if ((prot & PROT_EXEC) != 0)
    {
        protect = ((prot & PROT_WRITE) != 0) ? PAGE_EXECUTE_READWRITE :
            PAGE_EXECUTE_READ;
    }
    else
    {
        protect = ((prot & PROT_WRITE) != 0) ? PAGE_READWRITE : PAGE_READONLY;
    }

    return protect;
}

static DWORD protect_file(int prot)
{
    DWORD desired_access = 0;

    if (prot == PROT_NONE)
        return desired_access;

    if ((prot & PROT_READ) != 0)
        desired_access |= FILE_MAP_READ;

    if ((prot & PROT_WRITE) != 0)
        desired_access |= FILE_MAP_WRITE;

    if ((prot & PROT_EXEC) != 0)
        desired_access |= FILE_MAP_EXECUTE;

    return desired_access;
}

/* public interface */

void* mmap(void* addr, size_t len, int prot, int flags, int fildes, oft__ off)
{
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4293)
#endif

    const DWORD access  = protect_file(prot);
    const DWORD protect = protect_page(prot);

    const oft__ max = off + (oft__)len;

    const DWORD max_lo  = large ? (DWORD)((max)       & MAXDWORD) : (DWORD)max;
    const DWORD max_hi  = large ? (DWORD)((max >> 32) & MAXDWORD) : (DWORD)0;
    const DWORD file_lo = large ? (DWORD)((off)       & MAXDWORD) : (DWORD)off;
    const DWORD file_hi = large ? (DWORD)((off >> 32) & MAXDWORD) : (DWORD)0;

#ifdef _MSC_VER
#pragma warning(pop)
#endif

    if (len == 0 || (flags & MAP_FIXED) != 0 || prot == PROT_EXEC)
    {
        errno = EINVAL;
        return MAP_FAILED;
    }

    const HANDLE handle = ((flags & MAP_ANONYMOUS) == 0) ?
        (HANDLE)_get_osfhandle(fildes) : INVALID_HANDLE_VALUE;

    if ((flags & MAP_ANONYMOUS) == 0 && handle == INVALID_HANDLE_VALUE)
    {
        errno = EBADF;
        return MAP_FAILED;
    }

    const HANDLE mapping = CreateFileMapping(handle, NULL, protect, max_hi,
        max_lo, NULL);

    if (mapping == NULL)
    {
        errno = last_error(EPERM);
        return MAP_FAILED;
    }

    const LPVOID map = MapViewOfFile(mapping, access, file_hi, file_lo, len);

    /* TODO: verify mapping handle may be closed here and then use the map. */
    if (map == NULL || CloseHandle(mapping) == FALSE)
    {
        errno = last_error(EPERM);
        return MAP_FAILED;
    }

    errno = 0;
    return map;
}

int munmap(void* addr, size_t len)
{
    if (UnmapViewOfFile(addr) != FALSE)
    {
        errno = 0;
        return 0;
    }

    errno = last_error(EPERM);
    return -1;
}

int madvise(void* addr, size_t len, int advice)
{
    /* Not implemented, but should not fail (optimization). */
    return 0;
}

int mprotect(void* addr, size_t len, int prot)
{
    DWORD old_protect = 0;
    const DWORD new_protect = protect_page(prot);

    if (VirtualProtect(addr, len, new_protect, &old_protect) != FALSE)
    {
        errno = 0;
        return 0;
    }

    errno = last_error(EPERM);
    return -1;
}

int msync(void* addr, size_t len, int flags)
{
    if (FlushViewOfFile(addr, len) != FALSE)
    {
        errno = 0;
        return 0;
    }

    errno = last_error(EPERM);
    return -1;
}

int mlock(const void* addr, size_t len)
{
    if (VirtualLock((LPVOID)addr, len) != FALSE)
    {
        errno = 0;
        return 0;
    }

    errno = last_error(EPERM);
    return -1;
}

int munlock(const void* addr, size_t len)
{
    if (VirtualUnlock((LPVOID)addr, len) != FALSE)
    {
        errno = 0;
        return 0;
    }

    errno = last_error(EPERM);
    return -1;
}

/* Calling fsync() does not necessarily ensure that the entry in the directory
   containing the file has also reached disk. For that an explicit fsync() on
   a file descriptor for the directory is also needed. */
int fsync(int fd)
{
    const HANDLE handle = (HANDLE)(_get_osfhandle(fd));

    if (FlushFileBuffers(handle) != FALSE)
    {
        errno = 0;
        return 0;
    }

    errno = last_error(EPERM);
    return -1;
}

/* www.gitorious.org/git-win32/mainline/source/9ae6b7513158e0b1523766c9ad4a1ad286a96e2c:win32/ftruncate.c */
int ftruncate(int fd, oft__ size)
{
    LARGE_INTEGER big;

    if (fd < 0)
    {
        errno = EBADF;
        return -1;
    }

    /* guard against overflow from unsigned to signed */
    if (size >= (uint64_t)(large ? MAXINT64 : MAXINT32))
    {
        errno = EFBIG;
        return -1;
    }

    /* unsigned to signed, splits to high and low */
    big.QuadPart = (LONGLONG)size;

    const HANDLE handle = (HANDLE)_get_osfhandle(fd);
    const BOOL ok = SetFilePointerEx(handle, big, NULL, FILE_BEGIN);

    if (!ok || SetEndOfFile(handle) == FALSE)
    {
        errno = last_error(EIO);
        return -1;
    }

    errno = 0;
    return 0;
}

#endif
