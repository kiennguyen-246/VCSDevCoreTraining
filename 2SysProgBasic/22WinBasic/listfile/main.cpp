#define UNICODE
#define _UNICODE

#include <iostream>
#include <stdio.h>
#include <windows.h>
#include <winbase.h>
#include <tchar.h>
#include <fcntl.h>
#include <fstream>

int _tmain(int argc, _TCHAR *argv[])
{
    _setmode(_fileno(stdout), 0x00020000);
    _TCHAR *lpPwd = new _TCHAR[MAX_PATH];
    _tcscpy(lpPwd, argv[1]);
    _tcscat(lpPwd, __T("\\*"));
    // auto outFileHandle = CreateFile(
    //     _T("listdir.out"),
    //     GENERIC_READ | GENERIC_WRITE,
    //     FILE_SHARE_READ | FILE_SHARE_WRITE,
    //     NULL,
    //     OPEN_ALWAYS,
    //     FILE_ATTRIBUTE_NORMAL,
    //     NULL);
    try {
        std::wofstream fo;
        fo.open("listdir.out", std::ios_base::binary);
        SetCurrentDirectory(argv[1]);
        WIN32_FIND_DATA findData;
        HANDLE searchHandle = FindFirstFile(lpPwd, &findData);
        _TCHAR *fileName = new _TCHAR[MAX_PATH];
        _tcscpy(fileName, findData.cFileName);
        _tcscat(fileName, __T("\n"));
        DWORD nBytesWritten;
        _putws(fileName);
        // WriteFile(
        //     outFileHandle,
        //     &fileName,
        //     sizeof(fileName),
        //     &nBytesWritten,
        //     NULL
        // );
        fo << std::wstring(fileName);
        int count = 0;
        while (FindNextFile(searchHandle, &findData)) {
            if (++count == 7) break;
            _tcscpy(fileName, findData.cFileName);
            _tcscat(fileName, __T("\n"));
            _putws(fileName);
            fo << std::wstring(fileName);
            // WriteFile(
            //     outFileHandle,
            //     &fileName,
            //     sizeof(fileName),
            //     &nBytesWritten,
            //     NULL
            // );
        }
        FindClose(searchHandle);
        // CloseHandle(outFileHandle);
        fo.close();
    } catch (std::ios_base::failure &e) {
        std::cerr << e.what();
    }
    
    // _putws(lpPwd);
    return 0;
}