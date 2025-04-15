#include "DriverData.h"

CONST WCHAR kTargetPaths[MAX_TARGET_PATH][260] = {
    L"\\a\\a.txt", L"\\a", L"\\a\\DKOMUM.exe", L"\\DKOMUM.exe"};
CONST ACCESS_MASK kAllowedAccess = SYNCHRONIZE | FILE_READ_ATTRIBUTES |
                                   FILE_EXECUTE | FILE_READ_EA | READ_CONTROL;

DRIVER_DATA driverData;