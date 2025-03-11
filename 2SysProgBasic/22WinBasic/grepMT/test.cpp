#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <future>
#include <windows.h>

std::mutex mtx;
std::wofstream fo;

void writeFile(int id) {
    mtx.lock();
    fo.open("test.out", std::ios::app);
    fo << L"File is written by thread no " << id << L"\n";
    fo.close();
    mtx.unlock();
}

int wmain() {
    std::thread t1(writeFile, 1);
    std::thread t2(writeFile, 2);
    std::thread t3(writeFile, 3);
    std::thread t4(writeFile, 4);
    std::thread t5(writeFile, 5);
    std::thread t6(writeFile, 6);
    auto future7 = std::async(writeFile, 7);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    t6.join();
    future7.get();

    return 0;
}