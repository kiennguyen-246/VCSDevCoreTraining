#ifndef COMMON_GUARD
#define COMMON_GUARD

#include <fltkernel.h>
#include "public/public.h"

#define DbgPrint(x, ...) DbgPrint("[MiniFilter]" x, __VA_ARGS__)
#define MAX_TIMEOUT 50000000

typedef struct _MFLT_DATA {
  // The driver object
  PDRIVER_OBJECT pDriverObject;

  // The filter obtained from FltRegisterFilter
  PFLT_FILTER pFilter;

  // The server port. User mode connect to this
  PFLT_PORT pServerPort;

  // The client port. Only one user mode application is allowed at a time
  PFLT_PORT pClientPort;

  //KSPIN_LOCK kslRecordBufferLock;

  //CHAR pcOutputBuffer[MAX_BUFFER_SIZE];

  volatile BOOLEAN bIsComPortClosed;

  //LIST_ENTRY leOutputBufferList;
} MFLT_DATA, *PMFLT_DATA;

#endif  // COMMON_GUARD