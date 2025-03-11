#ifndef NPSERVERSVC_GUARD
#define NPSERVERSVC_GUARD
#include <iostream>
#include <fstream>
#include <future>
#include <chrono>
#include <ctime>
#include <windows.h>
#include <io.h>
#include <fcntl.h>

const int MAX_BUFFER_SIZE = 1024;
const int UPDATE_TIME = 1000;
const int MAX_RETRIES = 60;
const int MAX_CLIENTS = 3;

static bool shutdownFlag = false, pauseFlag = false;
static int eventId = 0;
static SERVICE_STATUS svcStatus;
static SERVICE_STATUS_HANDLE svcStatusHandle;

static std::wstring svcName = L"NPServerSvc";
static std::wstring logFileName = L"C:\\Users\\kiennd19\\Documents\\code\\winsysprog_learn\\commandServer\\Logs\\NPServerSvc.log";
static std::wofstream logFo;

void WINAPI svcMain(int argc, LPWSTR argv[]);
int svcSpecific(int argc, LPWSTR argv[]);
void WINAPI svcCtrlHandler(DWORD dwControl);
bool svcStatusUpdate(const int &newStatus, const int &checkPointMod);
bool svcLogEvent(const std::wstring &msg, const int &eventType);
bool svcLogEvent(const std::wstring &msg);
bool svcLogInit();
bool svcLogClose();
#endif