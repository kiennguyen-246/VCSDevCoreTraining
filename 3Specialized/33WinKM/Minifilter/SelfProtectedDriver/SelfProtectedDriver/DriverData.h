#pragma once
#include <fltKernel.h>

#include "Macros.h"

extern CONST UINT32 kMaxTargetPath;
extern CONST WCHAR kTargetPaths[][260];
extern CONST ACCESS_MASK kAllowedAccess;
extern CONST WCHAR kCOMPortName[];
extern CONST WCHAR kPassword[];

typedef struct _DRIVER_DATA {
  // The driver object
  PDRIVER_OBJECT pDriverObject;

  // The filter obtained from FltRegisterFilter
  PFLT_FILTER pFilter;

  // The server port. User mode connect to this
  PFLT_PORT pServerPort;

  // The client port. Only one user mode application is allowed at a time
  PFLT_PORT pClientPort;

  // KSPIN_LOCK kslRecordBufferLock;

  // CHAR pcOutputBuffer[MAX_BUFFER_SIZE];

  volatile BOOLEAN bIsComPortClosed;

  // LIST_ENTRY leOutputBufferList;

  BOOLEAN bIsUnlocked;
} DRIVER_DATA, *PDRIVER_DATA;

extern DRIVER_DATA driverData;