#include <iostream>
#include <windows.h>

int wmain(int argc, LPWSTR argv[]) {
    std::wstring fileName = argv[1];
    std::wstring fileNameNoExt = fileName.substr(0, fileName.size() - 4);
    int times = wcstol(argv[2], NULL, 10);
    for (int i = 1; i <= times; i ++) {
        std::wstring si = std::to_wstring(i);
        std::wstring newFileName = fileNameNoExt + si + L".txt";
        CopyFile(&fileName[0], &newFileName[0], 0);
    }
}