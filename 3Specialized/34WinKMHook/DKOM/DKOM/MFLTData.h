#pragma once
#include <fltkernel.h>

typedef struct _MFLT_DATA {
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
} MFLT_DATA, *PMFLT_DATA;