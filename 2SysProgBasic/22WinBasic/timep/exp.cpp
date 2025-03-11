#include <iostream>
#include <windows.h>

int main() {
    for (int i = 1; i <= 1000; i ++) {
        std::cout << i << "\n";
        Sleep(1000);
    }
    return 0;
}