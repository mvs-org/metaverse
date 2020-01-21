/**
* Copyright (c) 2019-2020 Parity Technologies (UK) Ltd.
* Copyright (c) 2019-2020 metaverse core developers (see MVS-AUTHORS)
*
* This file is part of metaverse-mvstray.
*
* metaverse-explorer is free software: you can redistribute it and/or
* modify it under the terms of the GNU Affero General Public License with
* additional permissions to the one published by the Free Software
* Foundation, either version 3 of the License, or (at your option)
* any later version. For more information see LICENSE.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Affero General Public License for more details.
*
* You should have received a copy of the GNU Affero General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#define WIN32_LEAN_AND_MEAN
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <Windows.h>
#include <Shellapi.h>
#include <Shlwapi.h>
#include <tlhelp32.h>

#include <SDKDDKVer.h>

#include "resource.h"

#pragma comment(lib, "shlwapi.lib")

#define MAX_LOADSTRING 100
#define IDM_EXIT 100
#define IDM_OPEN 101
#define IDM_AUTOSTART 102
#define WM_USER_SHELLICON WM_USER + 1
#define WM_TIMER_OPEN 107

HANDLE metaverseHandle = INVALID_HANDLE_VALUE;
DWORD metaverseProcId = 0;
NOTIFYICONDATA nidApp;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];
LPCWCHAR commandLineFiltered = L"";

LPCWSTR cMetaverseExe = _T("mvsd.exe");

ATOM MyRegisterClass(HINSTANCE hInstance);
bool InitInstance(HINSTANCE, int, LPWSTR cmdLine);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void KillMetaverse();
void OpenUI();
void FindRunningMetaverse();
bool MetaverseIsRunning();
bool AutostartEnabled();
void EnableAutostart(bool enable);

bool GetMetaverseExePath(TCHAR* dest, size_t destSize)
{
    if (!dest || MAX_PATH > destSize)
        return false;
    GetModuleFileName(NULL, dest, (DWORD)destSize);
    if (!PathRemoveFileSpec(dest))
        return false;
    return PathAppend(dest, _T("mvsd.exe")) == TRUE;
}

bool GetTrayExePath(TCHAR* dest, size_t destSize)
{
    if (!dest || MAX_PATH > destSize)
        return false;
    GetModuleFileName(NULL, dest, (DWORD)destSize);
    return true;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    CreateMutex(0, FALSE, _T("Local\\MetaverseTray"));
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        FindRunningMetaverse();
        if (metaverseHandle != INVALID_HANDLE_VALUE)
        {
            OpenUI();
            return 0;
        }
    }

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MVSTRAY, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow, lpCmdLine))
        return 0;

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MVSTRAY));
    MSG msg;
    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MVSTRAY));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MVSTRAY);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

void FindRunningMetaverse()
{
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    if ((snapshot != INVALID_HANDLE_VALUE) && Process32First(snapshot, &entry))
    {
        do
        {
            if (lstrcmp(entry.szExeFile, cMetaverseExe) == 0)
            {
                metaverseHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);
                metaverseProcId = entry.th32ProcessID;
                break;
            }
        } while (Process32Next(snapshot, &entry));
    }

    CloseHandle(snapshot);
}

bool InitInstance(HINSTANCE hInstance, int nCmdShow, LPWSTR cmdLine)
{
    FindRunningMetaverse();
    if (metaverseHandle == INVALID_HANDLE_VALUE)
    {
        // Launch Metaverse
        TCHAR path[MAX_PATH] = { 0 };
        if (!GetMetaverseExePath(path, MAX_PATH))
            return false;

        PROCESS_INFORMATION procInfo = { 0 };
        STARTUPINFO startupInfo = { sizeof(STARTUPINFO) };

        LPWSTR cmd = new WCHAR[lstrlen(cmdLine) + lstrlen(path) + 2];
        lstrcpy(cmd, path);
        lstrcat(cmd, _T(" "));
        lstrcat(cmd, cmdLine);
        BOOL succeed = CreateProcess(nullptr, cmd, nullptr, nullptr, false, CREATE_NO_WINDOW, nullptr, nullptr, &startupInfo, &procInfo);
        delete[] cmd;
        if (!succeed)
            return false;
        metaverseHandle = procInfo.hProcess;
        metaverseProcId = procInfo.dwProcessId;
    }

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, 0,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
        return false;

    HICON hMainIcon = LoadIcon(hInstance, (LPCTSTR)MAKEINTRESOURCE(IDI_MVSTRAY));

    nidApp.cbSize = sizeof(NOTIFYICONDATA); // sizeof the struct in bytes
    nidApp.hWnd = (HWND)hWnd;              //handle of the window which will process this app. messages
    nidApp.uID = IDI_MVSTRAY;           //ID of the icon that will appear in the system tray
    nidApp.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP; //ORing of all the flags
    nidApp.hIcon = hMainIcon; // handle of the Icon to be displayed, obtained from LoadIcon
    nidApp.uCallbackMessage = WM_USER_SHELLICON;
    LoadString(hInstance, IDS_CONTROL_METAVERSE, nidApp.szTip, MAX_LOADSTRING);
    Shell_NotifyIcon(NIM_ADD, &nidApp);

    SetTimer(hWnd, 0, 1000, nullptr);
    SetTimer(hWnd, WM_TIMER_OPEN, 1000, nullptr);

    return true;

}

void CreatePopMemu(HWND hWnd)
{
    UINT uFlag = MF_BYPOSITION | MF_STRING;
    POINT lpClickPoint;
    GetCursorPos(&lpClickPoint);
    HMENU hPopMenu = CreatePopupMenu();
    InsertMenu(hPopMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, IDM_OPEN, _T("Open"));
    InsertMenu(hPopMenu, 0xFFFFFFFF, MF_SEPARATOR | MF_BYPOSITION, 0, nullptr);
    InsertMenu(hPopMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, IDM_AUTOSTART, _T("Start at Login"));
    InsertMenu(hPopMenu, 0xFFFFFFFF, MF_SEPARATOR | MF_BYPOSITION, 0, nullptr);
    InsertMenu(hPopMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, IDM_EXIT, _T("Exit"));
    CheckMenuItem(hPopMenu, IDM_AUTOSTART, AutostartEnabled() ? MF_CHECKED : MF_UNCHECKED);

    SetForegroundWindow(hWnd);
    TrackPopupMenu(hPopMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_BOTTOMALIGN, lpClickPoint.x, lpClickPoint.y, 0, hWnd, NULL);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_USER_SHELLICON:
        // systray msg callback
        switch (LOWORD(lParam))
        {
        case WM_LBUTTONDOWN:
            OpenUI();
            break;
        case WM_RBUTTONDOWN:
            CreatePopMemu(hWnd);
            return 0;
        }
        break;
    case WM_COMMAND:
        // Parse the menu selections:
        switch (LOWORD(wParam))
        {
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        case IDM_OPEN:
            OpenUI();
            break;
        case IDM_AUTOSTART:
            EnableAutostart(!AutostartEnabled());
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    case WM_DESTROY:
        Shell_NotifyIcon(NIM_DELETE, &nidApp);
        KillMetaverse();
        PostQuitMessage(0);
        break;
    case WM_TIMER:
        switch (LOWORD(wParam))
        {
        case WM_TIMER_OPEN:
            if (MetaverseIsRunning()) {
                KillTimer(hWnd, WM_TIMER_OPEN);
                OpenUI();
            }
            break;
        default: // heartbeat
            if (!MetaverseIsRunning()) {
                DestroyWindow(hWnd);
            }
        }
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void KillMetaverse()
{
    DWORD procId = metaverseProcId;
    //This does not require the console window to be visible.
    if (AttachConsole(procId))
    {
        // Disable Ctrl-C handling for our program
        SetConsoleCtrlHandler(nullptr, true);
        GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0);
        FreeConsole();

        //Re-enable Ctrl-C handling or any subsequently started
        //programs will inherit the disabled state.
        SetConsoleCtrlHandler(nullptr, false);
    }
    WaitForSingleObject(metaverseHandle, INFINITE);
}

bool MetaverseIsRunning()
{
    return WaitForSingleObject(metaverseHandle, 0) == WAIT_TIMEOUT;
}

void OpenUI()
{
    // Launch Metaverse
    const TCHAR szOperation[] = _T("open");
    const TCHAR szAddress[] = _T("http://127.0.0.1:8820/");
    ShellExecute(NULL, szOperation, szAddress, NULL, NULL, SW_SHOWNORMAL);
}

bool AutostartEnabled() {
    HKEY hKey;
    LONG lRes = RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_READ, &hKey);
    if (lRes != ERROR_SUCCESS)
        return false;

    WCHAR szBuffer[512];
    DWORD dwBufferSize = sizeof(szBuffer);
    ULONG nError;
    nError = RegQueryValueExW(hKey, L"Metaverse", 0, nullptr, (LPBYTE)szBuffer, &dwBufferSize);
    if (ERROR_SUCCESS != nError)
        return false;
    return true;
}

void EnableAutostart(bool enable) {
    HKEY hKey;
    LONG lRes = RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_WRITE, &hKey);
    if (lRes != ERROR_SUCCESS)
        return;

    if (enable)
    {
        LPWSTR args = new WCHAR[lstrlen(commandLineFiltered) + MAX_PATH + 2];
        if (GetTrayExePath(args, MAX_PATH))
        {
            lstrcat(args, L" ");
            lstrcat(args, commandLineFiltered);
            RegSetValueEx(hKey, L"Metaverse", 0, REG_SZ, (LPBYTE)args, MAX_PATH);
        }
        delete[] args;
    }
    else
    {
        RegDeleteValue(hKey, L"Metaverse");
    }
}
