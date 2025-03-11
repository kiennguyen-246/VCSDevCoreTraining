#include <iostream>
#include <string>
#include <fstream>
#include <cstdio>
#include <windows.h>
#include <io.h>
#include <fcntl.h>

const int MAX_BUFFER_SIZE = 1024;

int wmain(int argc, LPWSTR argv[])
{
    _setmode(_fileno(stdout), _O_WTEXT);
    std::wstring pipeName = L"\\\\.\\pipe\\cmdPipe";
    bool quitFlag = 0;
    std::wcout << L"Pipe name" << pipeName << L"\n";
    HANDLE pipeHandle = INVALID_HANDLE_VALUE;
    while (pipeHandle == INVALID_HANDLE_VALUE)
    {
        WaitNamedPipe(&pipeName[0], NMPWAIT_WAIT_FOREVER);
        // std::wcout << L"Connection to the server has been established\n";
        pipeHandle = CreateFile(
            &pipeName[0],
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL);
        auto error = GetLastError();
        switch(error) {
            case ERROR_FILE_NOT_FOUND:
                std::wcout << "Cannot find the pipe name.\n";
                break;
            case ERROR_ACCESS_DENIED:
                std::wcout << "Cannot access the pipes.\n";
                break;
            default:
                break;
        }
    }
    DWORD npMode = PIPE_READMODE_MESSAGE | PIPE_WAIT;
    SetNamedPipeHandleState(pipeHandle, &npMode, NULL, NULL);
    if (pipeHandle != INVALID_HANDLE_VALUE && !quitFlag)
    {
        BYTE buffer[MAX_BUFFER_SIZE];
        DWORD dwRead = 0;
        DWORD dwWritten = 0;
        while (!quitFlag)
        {
            std::wstring cmd;
            std::wcout << L"NPClient>> ";
            getline(std::wcin, cmd);
            WriteFile(pipeHandle, &cmd[0], (DWORD)cmd.size() * 2, &dwWritten, NULL);

            while (ReadFile(pipeHandle, buffer, sizeof(buffer) - 2, &dwRead, NULL))
            {
                LPWSTR str = (LPWSTR)buffer;
                str[dwRead / 2] = L'\0';
                if (str[dwRead / 2 - 1] != L'\n')
                {
                    break;
                }
                std::wcout << std::wstring(str);
            }
            std::wcout << "\n";
            if (cmd == L"quit")
            {
                quitFlag = 1;
            }
        }
    }
    // std::wcout << L"Disconnected from the server\n";
    CloseHandle(pipeHandle);

    return 0;
}