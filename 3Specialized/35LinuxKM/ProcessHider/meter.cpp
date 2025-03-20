#include <iostream>
#include <unistd.h>

int main() {
    int iTime = 0;
    while (true) {
        sleep(10);
        iTime += 10;
        printf("%d seconds has passed\n", iTime);
    }
}