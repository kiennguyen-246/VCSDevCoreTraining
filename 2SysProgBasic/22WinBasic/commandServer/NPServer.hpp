#ifndef NPSERVER_GUARD
#define NPSERVER_GUARD
#include <iostream>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <future>
#include <windows.h>
#include <io.h>
#include <fcntl.h>

volatile bool stopFlag;

const int MAX_BUFFER_SIZE = 1024;
const int MAX_RETRIES = 60;
const int MAX_CLIENTS = 3;
const int CONNECTION_WAIT_TIMEOUT = 10000;

bool serverThreadFunc(HANDLE pipeHandle, const std::wstring &tmpFileName);
#endif