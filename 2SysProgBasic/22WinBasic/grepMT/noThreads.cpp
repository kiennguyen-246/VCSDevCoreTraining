#include <iostream>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <vector>
#include <utility>
#include <chrono>
#include <iomanip>
#include <windows.h>
#include <fcntl.h>

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
    std::vector<std::pair<int, int> > result[(int)fileNameList.size()];
    int fileId = 0;
    for (auto curFileName: fileNameList) {
        try {
            findInFile(grepStr, curFileName, result[fileId]);
            ++fileId;
        } catch (std::ios_base::failure &e) {
            std::cout << e.what() << "\n";
        }   
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
    return 0;
}