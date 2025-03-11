#include <iostream>
#include <windows.h>
#include <io.h>
#include <fcntl.h>

int wmain()
{
    _setmode(_fileno(stdout), _O_WTEXT);
    STARTUPINFO startupInfo;
    GetStartupInfo(&startupInfo);
    PROCESS_INFORMATION procInfo;
    std::wstring cmd = L"NPServer";
    CreateProcessW(
            NULL,
            &cmd[0],
            NULL,
            NULL,
            TRUE,
            0,
            NULL,
            NULL,
            &startupInfo,
            &procInfo);
    return 0;
}