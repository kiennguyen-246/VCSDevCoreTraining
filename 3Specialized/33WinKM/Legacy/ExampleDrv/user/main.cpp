#include <iostream>
#include <windows.h>
#include <io.h>
#include <fcntl.h>

const std::wstring DRIVER_NAME = L"HelloWorldKMDF2";
const std::wstring DRIVER_PATH = L"C:\\DriverTest\\Drivers\\" + DRIVER_NAME + L".sys";

#define IOCTL_SAMPLE_BUFFERED_IO                          \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, \
             FILE_READ_DATA | FILE_WRITE_DATA)

int wmain(int argc, LPWSTR argv[])
{
    // _setmode(_fileno(stdout), _O_WTEXT);

    // auto hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    // if (hSCM)
    // {
    //     std::wcout << L"SCM successfully loaded\n";
    //     auto hSvc = CreateService(
    //         hSCM,
    //         DRIVER_NAME.c_str(),
    //         (DRIVER_NAME + L"Driver").c_str(),
    //         SERVICE_START | DELETE | SERVICE_STOP,
    //         SERVICE_KERNEL_DRIVER,
    //         SERVICE_DEMAND_START,
    //         SERVICE_ERROR_IGNORE,
    //         DRIVER_PATH.c_str(),
    //         NULL, NULL, NULL, NULL, NULL);
    //     if (!hSvc)
    //     {
    //         hSvc = OpenService(hSCM, DRIVER_NAME.c_str(), SERVICE_START | DELETE | SERVICE_STOP);
    //     }
    //     if (hSvc)
    //     {
    //         StartService(hSvc, 0, NULL);
    //         std::wcout << L"Successfully started the service\n";
    //         std::wcout << L"Press Enter to stop the service\n";
    //         getchar();
    //         SERVICE_STATUS ss;
    //         ControlService(hSvc, SERVICE_CONTROL_STOP, &ss);
    //         DeleteService(hSvc);
    //         CloseServiceHandle(hSvc);
    //     }
    //     CloseServiceHandle(hSCM);
    // }
    // else
    // {
    //     std::wcout << "Please open this program as administrator\n";
    //     getchar();
    // }

    HANDLE hFile;

    std::wcout << L"Ready to make your screen blue?\n";
    getchar();

    hFile = CreateFile(L"\\\\.\\ExampleDrv",
                       GENERIC_READ | GENERIC_WRITE, 0, NULL,
                       OPEN_EXISTING, 0, NULL);
    std::wcout << L"Called CreateFile() successfull\n";
    getchar();

    // if(hFile)
    // {

    // }
    if (hFile != INVALID_HANDLE_VALUE)
    {
        // std::wcout << L"Successfully created handle for file in the device\n";
        // WriteFile(hFile, L"Hello from user mode!",
        //         sizeof(L"Hello from user mode!"), &dwReturn, NULL);
        // std::wcout << L"Successfully write something to the file\n";
        // getchar();
        // CloseHandle(hFile);

        // std::wcout << L"Successfully created handle for file in the device\n";
        // BYTE buffer[1024];
        // DWORD dwRead = 0;
        // while (ReadFile(hFile, buffer, sizeof(buffer) - 2, &dwRead, NULL))
        // {
        //     LPWSTR str = (LPWSTR)buffer;
        //     str[dwRead / 2] = L'\0';
        //     if (str[dwRead / 2 - 1] != L'\n')
        //     {
        //         break;
        //     }
        //     std::wcout << std::wstring(str);
        // }
        // getchar();
        // CloseHandle(hFile);

        std::wcout << L"Successfully created handle for file in the device\n";
        BYTE inBuffer[1024], outBuffer[1024];
        DWORD dwRead = 0;
        ZeroMemory(inBuffer, sizeof(inBuffer));
        ZeroMemory(outBuffer, sizeof(outBuffer));
        WCHAR lpwsMsg[] = L"** Hello from User Mode Buffered I/O";
        DWORD dwMsgSize = wcslen(lpwsMsg) * sizeof(WCHAR);
        memcpy(inBuffer, lpwsMsg, dwMsgSize);
        DeviceIoControl(hFile,
                        IOCTL_SAMPLE_BUFFERED_IO,
                        inBuffer,
                        sizeof(inBuffer),
                        outBuffer,
                        sizeof(outBuffer),
                        &dwRead,
                        NULL);
        std::wcout << std::wstring((LPWSTR)outBuffer);
        getchar();
        CloseHandle(hFile);
    }
    else
    {
        auto error = GetLastError();
        switch (error)
        {
        case ERROR_FILE_NOT_FOUND:
            std::wcout << L"Cannot find the file.\n";
            break;
        case ERROR_ACCESS_DENIED:
            std::wcout << L"Cannot access the file.\n";
            break;
        default:
            std::wcout << L"GetLastError() returns " << error << L"\n";
            break;
        }
    }

    return 0;
}