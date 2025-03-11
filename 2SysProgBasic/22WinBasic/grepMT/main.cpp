#include <iostream>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <vector>
#include <utility>
#include <chrono>
#include <iomanip>
#include <thread>
#include <windows.h>
#include <process.h>
#include <fcntl.h>

const int MAX_FILE_NUMBER = 20;
// static std::vector<std::pair<int, int> > result[MAX_FILE_NUMBER];

bool findInFile(const std::wstring &findStr, const std::wstring &fileName, std::vector<std::pair<int, int> > &result) {
    std::wifstream fi(&fileName[0]);
    std::wstring ln;
    int lnCnt = 0;
    while (getline(fi, ln)) {
        ++lnCnt;
        std::wstringstream ssln(ln);
        std::wstring str;
        int pos = 0;
        while (ssln >> str) {
            // while (str.back() == '.') str.pop_back();
            if (str == findStr) {
                result.push_back({lnCnt, pos});
            }
            pos += str.length() + 1;
        }
    }
    fi.close();
    return 1;
}

struct ThreadArgs{
    std::wstring findStr;
    std::wstring fileName;
    std::vector<std::pair<int, int> > *result;

    ThreadArgs() {

    }

    ThreadArgs(const std::wstring &__findStr, const std::wstring &__fileName, std::vector<std::pair<int, int> > &__result) {
        findStr = __findStr;
        fileName = __fileName;
        result = &__result;
    }
};


int wmain(int argc, LPWSTR argv[]) {
    _setmode( _fileno(stdout), _O_WTEXT );
    auto start = std::chrono::high_resolution_clock::now();
    if (argc < 2) {
        std::wcout << "No string";
        return 0;
    }
    if (argc < 3) {
        std::wcout << "No file\n";
        return 0;
    }
    std::wstring grepStr = argv[1];
    std::vector<std::wstring> fileNameList;
    fileNameList.clear();
    for (int i = 2; i < argc; i ++) {
        std::wstring curFileName = argv[i];
        fileNameList.push_back(curFileName);
    }

    // std::vector<int> v[4];
    // v[0].assign({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22});
    // v[1].assign({6, 7, 8, 9});
    // v[2].assign({10, 11, 12, 13, 14, 15});

    std::vector<std::pair<int, int> > result[(int)fileNameList.size()];
    std::thread threads[(int)fileNameList.size()];
    for (int i = 0; i < (int)fileNameList.size(); i++) {
        auto curFileName = fileNameList[i];
        threads[i] = std::thread(findInFile, grepStr, curFileName, result[i]);
    }
    for (int i = 0; i < (int)fileNameList.size(); i++) {
        threads[i].join();
    }

    std::wofstream fo(L"result.out");
    for (int i = 0; i < (int)fileNameList.size(); i++) {
        fo << fileNameList[i] << L"\n";
        for (auto p: result[i]) {
            fo << p.first << L":" << p.second << L"\n";
        }
    }
    fo.close();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elp = end - start;
    std::wcout << std::setw(9) << L"Time elapsed: " << elp << L"\n";
    // delete[] result;
    return 0;
}