#ifndef INCLUDE_GUARD
#define INCLUDE_GUARD

#include <wdm.h>

// #define __USE_DIRECT__
#define __USE_BUFFERED__

#ifdef __USE_DIRECT__
#define IO_TYPE DO_DIRECT_IO
#endif

#ifdef __USE_BUFFERED__
#define IO_TYPE DO_BUFFERED_IO
#endif

#ifndef IO_TYPE
#define IO_TYPE 0
#endif

NTSTATUS handleUnsupportedFunction(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);
NTSTATUS handleCloseFile(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);
NTSTATUS handleCreateFile(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);
NTSTATUS handleDeviceIoControl(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);
NTSTATUS handleReadFile(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);
NTSTATUS handleWriteFile(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);

#pragma alloc_text(PAGE, handleUnsupportedFunction)
#pragma alloc_text(PAGE, handleCreateFile)
#pragma alloc_text(PAGE, handleReadFile)
#pragma alloc_text(PAGE, handleWriteFile)
#pragma alloc_text(PAGE, handleDeviceIoControl)
#pragma alloc_text(PAGE, handleCloseFile)

// This macro is initialized backwards, so the name should be 'Exp '
#define EXAMPLE_POOL_TAG ' pxE'
#define MIN(x, y) (x <= y ? x : y)

typedef struct _EXAMPLE_LIST {
  // Pointer to the next structure in the list
  struct _EXAMPLE_LIST *pNext;

  // Name of the pipe
  UNICODE_STRING usPipeName;

  // The wchar* version of the pipe name
  WCHAR pwcPipeName[256];

  // Current number of reference to this pipe.
  // One is added when a new pipe with the same name is created
  // One is subtracted when a pipe is released
  UINT32 uiRefCount;

  // Starting index of the written data in the circular buffer
  // This value may only be modified by read operations
  UINT32 uiStartIndex;

  // Ending index of the written data in the circular buffer
  // This value may only be modified by write operations
  UINT32 uiStopIndex;

  // Kernel mutex for threads using the current pipe context
  KMUTEX kmInstanceBufferMutex;

  // The circular data buffer
  CHAR pcCircularBuffer[2000];
} EXAMPLE_LIST, *PEXAMPLE_LIST;

// The user-defined driver context
typedef struct _EXAMPLE_DEVICE_CONTEXT {
  // List of pipe context instances
  PEXAMPLE_LIST pExampleList;

  // Kernel mutex for all pipe contexts
  KMUTEX kmListMutex;
} EXAMPLE_DEVICE_CONTEXT, *PEXAMPLE_DEVICE_CONTEXT;
#endif