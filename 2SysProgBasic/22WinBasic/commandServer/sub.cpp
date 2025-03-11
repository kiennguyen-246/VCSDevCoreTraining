#include <iostream>
#include <windows.h>

int wmain(int argc, LPWSTR argv[]) {
    LPWSTR pEnd;
    std::wcout << wcstol(argv[1], &pEnd, 10) - wcstol(argv[2], &pEnd, 10);
    return 0;
}