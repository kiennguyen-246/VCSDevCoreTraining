#include <iostream>
#include <cstdio>
#include <iomanip>
#include <windows.h>
#include <fcntl.h>

int wmain() {
    _setmode( _fileno(stdout), _O_WTEXT );
    try {
        auto procHandle = OpenProcess(
            SYNCHRONIZE,
            TRUE,
            4224
        );
        // union {
        //     long long li;
        //     FILETIME ft;
        // } cre, ext, elp;
        FILETIME cre, ext, elp, krn, usr, now;
        SYSTEMTIME elpSys, krnSys, usrSys, nowSys;
        GetSystemTime(&nowSys);
        SystemTimeToFileTime(&nowSys, &now);
        GetProcessTimes(procHandle, &cre, &ext, &krn, &usr);
        ULARGE_INTEGER creULI, nowULI, elpULI;
        creULI.HighPart = cre.dwHighDateTime;
        creULI.LowPart = cre.dwLowDateTime;
        nowULI.HighPart = now.dwHighDateTime;
        nowULI.LowPart = now.dwLowDateTime;
        elpULI.QuadPart = nowULI.QuadPart - creULI.QuadPart;
        elp.dwHighDateTime = elpULI.HighPart;
        elp.dwLowDateTime = elpULI.LowPart;
        FileTimeToSystemTime(&elp, &elpSys);

        // GetProcessTimes(procHandle, &cre.ft, &ext.ft, &krn, &usr);
        // elp.li = cre.li - ext.li;
        // FileTimeToSystemTime(&elp.ft, &elpSys);
        // FileTimeToSystemTime(&krn, &krnSys);
        // FileTimeToSystemTime(&usr, &usrSys);
        // std::cerr << cre.dwHighDateTime << " " << cre.dwLowDateTime << "\n";
        // std::cerr << ext.dwHighDateTime << " " << ext.dwLowDateTime << "\n";
        wprintf(L"Process time: %02d:%02d:%02d:%02d", elpSys.wHour, elpSys.wMinute, elpSys.wSecond, elpSys.wMilliseconds);

        CloseHandle(procHandle);


    } catch (std::bad_alloc &e) {
        std::cerr << e.what();
    } 
    
    return 0;
}