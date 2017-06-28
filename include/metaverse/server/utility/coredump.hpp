#ifndef CORE_DUMP_HPP
#define CORE_DUMP_HPP

#include <Windows.h>
#include <Dbghelp.h>

#pragma comment( lib, "Dbghelp.lib" )

namespace libbitcoin {
namespace server {

LONG WINAPI ExpFilter(struct _EXCEPTION_POINTERS *pExp)
{
	SYSTEMTIME systime;
	GetLocalTime(&systime);
	systime.wDayOfWeek;

	WORD;
	char path[128] = { 0 };
	sprintf_s(path, ".\\mvsd%d%02d%02d%02d%02d%02d.dmp", systime.wYear, 
		systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, 
		systime.wSecond);
	HANDLE hFile = ::CreateFile(
		path,
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

		::MiniDumpWriteDump(
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


/// catch unhandle exception save to dump.
void StartUnhandledExceptionFilter()
{
	::SetUnhandledExceptionFilter(ExpFilter);
}

} //namespace server
} //namespace libbitcoin
#endif