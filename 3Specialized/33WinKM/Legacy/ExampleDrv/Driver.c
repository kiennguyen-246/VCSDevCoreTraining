#include <wdm.h>

#define DbgPrint(x, ...) \
  DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, x, __VA_ARGS__)

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

#define IOCTL_SAMPLE_DIRECT_IN_IO                        \
  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_IN_DIRECT, \
           FILE_READ_DATA | FILE_WRITE_DATA)

#define IOCTL_SAMPLE_DIRECT_OUT_IO                        \
  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_OUT_DIRECT, \
           FILE_READ_DATA | FILE_WRITE_DATA)

#define IOCTL_SAMPLE_BUFFERED_IO                        \
  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, \
           FILE_READ_DATA | FILE_WRITE_DATA)

#define IOCTL_SAMPLE_NEITHER_IO                        \
  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_NEITHER, \
           FILE_READ_DATA | FILE_WRITE_DATA)

NTSTATUS handleUnsupportedFunction(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);
NTSTATUS handleCloseFile(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);
NTSTATUS handleCreateFile(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);
NTSTATUS handleDeviceIoControl(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);
NTSTATUS handleReadFile(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);
NTSTATUS handleWriteFile(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject,
                     PUNICODE_STRING pUnicodeString);
VOID driverUnload(PDRIVER_OBJECT pDriverObject);

NTSTATUS handleSampleIoctlBufferedIo(PIRP pIrp,
                                     PIO_STACK_LOCATION pIoStackLocation,
                                     UINT32* pwDataWritten);

BOOLEAN isStringTerminated(PCHAR pStr, UINT32 uiStrLen);

#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, handleUnsupportedFunction)
#pragma alloc_text(PAGE, handleCreateFile)
#pragma alloc_text(PAGE, handleReadFile)
#pragma alloc_text(PAGE, handleWriteFile)
#pragma alloc_text(PAGE, handleDeviceIoControl)
#pragma alloc_text(PAGE, handleSampleIoctlBufferedIo)
#pragma alloc_text(PAGE, handleCloseFile)
#pragma alloc_text(PAGE, driverUnload)
#pragma alloc_text(PAGE, isStringTerminated)

/**
 * Check if the string is terminated (end with a '\0')
 *
 * @param pStr Pointer to the string
 * @param uiStrLen Length of the string
 *
 * @return TRUE if the string is terminated
 */
BOOLEAN isStringTerminated(PCHAR pStr, UINT32 uiStrLen) {
  BOOLEAN bStringIsTerminated = FALSE;
  UINT32 uiIndex = 0;

  while (uiIndex < uiStrLen && bStringIsTerminated == FALSE) {
    // DbgPrint(pStr[uiIndex]);
    if (pStr[uiIndex] == '\0' && pStr[uiIndex + 1] == '\0') {
      bStringIsTerminated = TRUE;
    } else {
      if (!bStringIsTerminated) {
      }
      uiIndex += 2;
    }
  }
  // DbgPrint("\n");
  return bStringIsTerminated;
}

NTSTATUS handleUnsupportedFunction(PDEVICE_OBJECT pDeviceObject, PIRP pIrp) {
  NTSTATUS NtStatus = STATUS_NOT_SUPPORTED;
  DbgPrint("unsupportedFunction() Called.\r\n");
  return NtStatus;
}

NTSTATUS handleCloseFile(PDEVICE_OBJECT pDeviceObject, PIRP pIrp) {
  NTSTATUS NtStatus = STATUS_SUCCESS;
  DbgPrint("close() Called.\r\n");
  return NtStatus;
}

NTSTATUS handleCreateFile(PDEVICE_OBJECT pDeviceObject, PIRP pIrp) {
  NTSTATUS NtStatus = STATUS_SUCCESS;
  DbgPrint("create() Called.\r\n");
  return NtStatus;
}

/**
 * Communicating with the user mode DeviceIoControl() function.
 *
 * IOCTLs (IO Control Codes) are 32-bit numbers exported by the driver to
 * communicate with the user mode application. They defines the access required
 * in order to issue the IOCTL as well as the methodd to be used when
 * transferring the data between the driver and the application. When the user
 * mode application call DeviceIoControl, the IOCTL will be included in the code
 * and passed directly to the driver.
 *
 * The bit layout of an IOCTL is:
 *
 * [Common |Device Type|Required Access|Custom|Function Code|Transfer Type]
 *
 *   31     30       16 15          14  13   12           2  1            0
 *
 *   Common          - 1 bit.  This is set for user-defined
 *                     device types.
 *
 *   Device Type     - This is the type of device the IOCTL
 *                     belongs to.  This can be user defined
 *                     (Common bit set).  This must match the
 *                     device type of the device object.
 *
 *   Required Access - FILE_READ_DATA, FILE_WRITE_DATA, etc.
 *                     This is the required access for the
 *                     device.
 *
 *   Custom          - 1 bit.  This is set for user-defined
 *                     IOCTL's.  This is used in the same
 *                     manner as "WM_USER".
 *
 *   Function Code   - This is the function code that the
 *                     system or the user defined (custom
 *                     bit set)
 *
 *   Transfer Type   - METHOD_IN_DIRECT, METHOD_OUT_DIRECT,
 *                     METHOD_NEITHER, METHOD_BUFFERED, This
 *                     the data transfer method to be used.
 *
 * @param pDriverObject Pointer to the current driver object
 * @param pIrp Pointer to the IRP structure
 *
 */
NTSTATUS handleDeviceIoControl(PDEVICE_OBJECT pDeviceObject, PIRP pIrp) {
  NTSTATUS NtStatus = STATUS_NOT_SUPPORTED;
  PIO_STACK_LOCATION pIoStackIrp = NULL;
  UINT32 dwDataWritten = 0;

  DbgPrint("handleDeviceIoControl() called\n");

  pIoStackIrp = IoGetCurrentIrpStackLocation(pIrp);

  if (pIoStackIrp) {
    switch (pIoStackIrp->Parameters.DeviceIoControl.IoControlCode) {
      case IOCTL_SAMPLE_DIRECT_IN_IO:
        break;
      case IOCTL_SAMPLE_DIRECT_OUT_IO:
        break;
      case IOCTL_SAMPLE_BUFFERED_IO:
        NtStatus =
            handleSampleIoctlBufferedIo(pIrp, pIoStackIrp, &dwDataWritten);
        break;
      case IOCTL_SAMPLE_NEITHER_IO:
        break;
    }
  }

  pIrp->IoStatus.Status = NtStatus;
  pIrp->IoStatus.Status = NtStatus;
  pIrp->IoStatus.Information = dwDataWritten;

  IoCompleteRequest(pIrp, IO_NO_INCREMENT);

  return NtStatus;
}

/**
 * Handle IOCTL using buffered IO method.
 *
 * @param pIrp Current IRP struct in process
 * @param pIoStackIrp Location of the I/O stack in the current IRP
 * @param puiDataWritten Number of bytes of data transfered from the user mode
 * application
 */
NTSTATUS handleSampleIoctlBufferedIo(PIRP pIrp, PIO_STACK_LOCATION pIoStackIrp,
                                     UINT32* puiDataWritten) {
  NTSTATUS NtStatus = STATUS_UNSUCCESSFUL;
  PCHAR pInputBuffer;
  PCHAR pOutputBuffer;
  PCWCHAR pReturnData = L"IOCTL - Buffered I/O From Kernel!";
  UINT32 uiDataSize = (UINT32)wcslen(pReturnData) * sizeof(WCHAR);

  DbgPrint("handleSampleIoctlBufferedIo() called\n");

  // Note that the input and output location is the same
  pInputBuffer = pIrp->AssociatedIrp.SystemBuffer;
  pOutputBuffer = pIrp->AssociatedIrp.SystemBuffer;

  if (pInputBuffer && pOutputBuffer) {
    if (isStringTerminated(
            pInputBuffer,
            pIoStackIrp->Parameters.DeviceIoControl.InputBufferLength)) {
      DbgPrint("Message form user mode: '%ws'\n", pInputBuffer);

      DbgPrint("%i => %i\n",
               pIoStackIrp->Parameters.DeviceIoControl.OutputBufferLength,
               uiDataSize);
      if (pIoStackIrp->Parameters.DeviceIoControl.OutputBufferLength >=
          uiDataSize) {
        RtlCopyMemory(pOutputBuffer, pReturnData, uiDataSize);
        *puiDataWritten = uiDataSize;
        NtStatus = STATUS_SUCCESS;
        DbgPrint("Transfering successful.\n");
      } else {
        *puiDataWritten = uiDataSize;
        NtStatus = STATUS_BUFFER_TOO_SMALL;
        DbgPrint("Transfering failed due to too small buffer.\n");
      }
    } else {
      DbgPrint("Transfering failed due to user message not terminated.\n");
    }
  } else {
    DbgPrint("Transfering failed due to buffer not found.\n");
  }

  return NtStatus;
}

/**
 * Communicating with the user mode ReadFile() function
 *
 * Supports 3 different input method: direct I/O, buffered I/O, neither
 *
 * @param pDriverObject Pointer to the current driver object
 * @param pIrp Pointer to the IRP structure
 */
NTSTATUS handleReadFile(PDEVICE_OBJECT pDeviceObject, PIRP pIrp) {
#ifdef __USE_DIRECT__

#else
#ifdef __USE_BUFFERED__
  NTSTATUS NtStatus = STATUS_BUFFER_TOO_SMALL;
  PIO_STACK_LOCATION pIoStackIrp = NULL;
  PWCHAR pReturnDataW = L"Hello from the kernel";
  // PCHAR pReturnData = (PCHAR)pReturnDataW;
  // UINT32 dwDataSize = sizeof(pReturnDataW);
  UINT32 dwDataSize = (INT)wcslen(pReturnDataW) * sizeof(WCHAR);
  UINT32 dwDataRead = 0;
  PCHAR pReadDataBuffer;

  DbgPrint("read() Called.\r\n");
  DbgPrint("Output string needed: %ws\r\n", pReturnDataW);
  DbgPrint("Data size is: %d\r\n", dwDataSize);

  pIoStackIrp = IoGetCurrentIrpStackLocation(pIrp);

  if (pIoStackIrp) {
    DbgPrint("Preparing to modify the buffer\n");
    pReadDataBuffer = (PCHAR)pIrp->AssociatedIrp.SystemBuffer;
    if (pReadDataBuffer && pIoStackIrp->Parameters.Read.Length >= dwDataSize) {
      DbgPrint("Started writing to the buffer\n");
      RtlCopyMemory(pReadDataBuffer, pReturnDataW, dwDataSize);
      dwDataRead = dwDataSize;
      NtStatus = STATUS_SUCCESS;
    }
  }

  pIrp->IoStatus.Status = NtStatus;
  pIrp->IoStatus.Information = dwDataRead;

  // Priority boost to make the thread waiting for this IRP to complete
  IoCompleteRequest(pIrp, IO_NO_INCREMENT);

  return NtStatus;
#else

#endif  //__USE_BUFFERED__
#endif  //__USE_DIRECT__
}

/**
 * Communicating with the user mode WriteFile() function
 *
 * Supports 3 different input method: direct I/O, buffered I/O, neither
 *
 * @param pDriverObject Pointer to the current driver object
 * @param pIrp Pointer to the IRP structure
 */
NTSTATUS handleWriteFile(PDEVICE_OBJECT pDeviceObject, PIRP pIrp) {
#ifdef __USE_DIRECT__

#else
#ifdef __USE_BUFFERED__
  NTSTATUS NtStatus = STATUS_SUCCESS;
  PIO_STACK_LOCATION pIoStackIrp = NULL;
  PCHAR pWriteDataBuffer;

  DbgPrint("write() Called. Mode: Buffered I/O \r\n");

  pIoStackIrp = IoGetCurrentIrpStackLocation(pIrp);

  if (pIoStackIrp) {
    // DbgPrint("Successfully get stack location\n");
    pWriteDataBuffer = (PCHAR)pIrp->AssociatedIrp.SystemBuffer;
    if (pWriteDataBuffer) {
      // DbgPrint("Access buffer successfully\n");
      if (isStringTerminated(pWriteDataBuffer,
                             pIoStackIrp->Parameters.Write.Length)) {
        // DbgPrint("String is terminated\n");
        DbgPrint("%ws", pWriteDataBuffer);
      } else {
        // DbgPrint("String is not terminated\n");
      }
    }
    DbgPrint("\n");
  }
  return NtStatus;
#else

#endif  //__USE_BUFFERED__
#endif  //__USE_DIRECT__
}

/**
 * Function used to unload the driver dynamically
 *
 * @param pDriverObject Pointer to the current driver object
 */
VOID driverUnload(PDRIVER_OBJECT pDriverObject) {
  UNICODE_STRING usDosDeviceName;

  DbgPrint("unload() Called\n");

  // Delete the symbolic link created in DriverEntry()
  RtlInitUnicodeString(&usDosDeviceName, L"\\DosDevice\\ExampleDrv");
  IoDeleteSymbolicLink(&usDosDeviceName);

  // Delete the device
  IoDeleteDevice(pDriverObject->DeviceObject);
}

/**
 * Entry point of the driver
 *
 * @param pDriverObject Pointer to the current driver object
 * @param pUnicodeString String points to the registry location that store the
 * information of the driver
 */
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject,
                     PUNICODE_STRING pRegistryPath) {
  NTSTATUS NtStatus = STATUS_SUCCESS;
  UINT32 uiIndex = 0;
  PDEVICE_OBJECT pDeviceObject = NULL;
  UNICODE_STRING usDriverName, usDosDeviceName;

  // Printing to the DebugView window
  DbgPrint("DriverEntry called\n");

  // Init unicode strings (same as strcat/wcscat)
  // Note that UNICODE_STRINGs do not have \0 at the end
  RtlInitUnicodeString(&usDriverName, L"\\Device\\ExampleDrv");
  RtlInitUnicodeString(&usDosDeviceName, L"\\DosDevices\\ExampleDrv");

  // Create the device driver
  NtStatus =
      IoCreateDevice(pDriverObject, 0, &usDriverName, FILE_DEVICE_UNKNOWN,
                     FILE_DEVICE_SECURE_OPEN, FALSE, &pDeviceObject);
  if (NtStatus != STATUS_SUCCESS) {
    DbgPrint("IoCreateDevice failed. Exitcode 0x%08x\n", NtStatus);
    // return NtStatus;
  }

  if (NtStatus == STATUS_SUCCESS) {
    // Define major functions used to communicate with the user mode
    // application
    for (uiIndex = 0; uiIndex < IRP_MJ_MAXIMUM_FUNCTION; ++uiIndex) {
      pDriverObject->MajorFunction[uiIndex] = handleUnsupportedFunction;
    }

    pDriverObject->MajorFunction[IRP_MJ_CLOSE] = handleCloseFile;
    pDriverObject->MajorFunction[IRP_MJ_CREATE] = handleCreateFile;
    pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = handleDeviceIoControl;
    pDriverObject->MajorFunction[IRP_MJ_READ] = handleReadFile;
    pDriverObject->MajorFunction[IRP_MJ_WRITE] = handleWriteFile;

    pDriverObject->DriverUnload = driverUnload;

    pDeviceObject->Flags |= IO_TYPE;  // type of IO
    pDeviceObject->Flags &=
        (~DO_DEVICE_INITIALIZING);  // remove the flag used to tell the IO
                                    // manager that the device is initialized

    // Create a symbolic link between DOS device name and NT device name
    IoCreateSymbolicLink(&usDosDeviceName, &usDriverName);
  }

  return NtStatus;
  // return STATUS_SUCCESS;
}