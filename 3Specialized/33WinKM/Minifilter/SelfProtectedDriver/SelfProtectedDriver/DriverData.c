#include "DriverData.h"

CONST UINT32 kMaxTargetPath = 10;
CONST WCHAR kTargetPaths[][260] = {
    L"\\a\\a.txt", L"\\a", L"\\a\\DKOMUM.exe", L"\\DKOMUM.exe"};
CONST ACCESS_MASK kAllowedAccess = SYNCHRONIZE | FILE_READ_ATTRIBUTES |
                                   FILE_EXECUTE | FILE_READ_EA | READ_CONTROL;
CONST WCHAR kCOMPortName[] = L"\\UnlockCOM";
CONST WCHAR kPassword[] = L"secret";

DRIVER_DATA driverData;