#ifndef CORE_DUMP_HPP
#define CORE_DUMP_HPP
#include <boost/format.hpp>

namespace libbitcoin {
namespace server {

#ifdef _MSC_VER

#include <Windows.h>
#include <Dbghelp.h>
#pragma comment( lib, "Dbghelp.lib" )

LONG WINAPI exp_filter(struct _EXCEPTION_POINTERS *pExp)
{
    SYSTEMTIME systime;
    GetLocalTime(&systime);
#ifdef UNICODE
    boost::wformat fmt(L".\\mvsd%d%02d%02d%02d%02d%02d.dmp");
#else
    boost::format fmt(".\\mvsd%d%02d%02d%02d%02d%02d.dmp");
#endif // UNICODE
    fmt % systime.wYear % systime.wMonth % systime.wDay % systime.wHour %
        systime.wMinute % systime.wSecond;
    HANDLE hFile = ::CreateFile(
        fmt.str().c_str(),
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (INVALID_HANDLE_VALUE != hFile)
    {
        MINIDUMP_EXCEPTION_INFORMATION einfo;
        einfo.ThreadId = ::GetCurrentThreadId();
        einfo.ExceptionPointers = pExp;
        einfo.ClientPointers = FALSE;

        MiniDumpWriteDump(
            ::GetCurrentProcess(),
            ::GetCurrentProcessId(),
            hFile,
            MiniDumpWithFullMemory,
            &einfo,
            NULL,
            NULL);
        ::CloseHandle(hFile);
    }

    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

/// catch unhandle exception save to dump.
void start_unhandled_exception_filter()
{
#ifdef _MSC_VER
    ::SetUnhandledExceptionFilter(exp_filter);
#endif
}

} //namespace server
} //namespace libbitcoin
#endif