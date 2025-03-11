#include "include.h"

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject,
                     PUNICODE_STRING pUnicodeString);
VOID driverUnload(PDRIVER_OBJECT pDriverObject);

#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, driverUnload)

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
  PEXAMPLE_DEVICE_CONTEXT pExampleDeviceContext;

  // Printing to the DebugView window
  DbgPrint("DriverEntry called\n");

  // Init unicode strings (same as strcat/wcscat)
  // Note that UNICODE_STRINGs do not have \0 at the end
  RtlInitUnicodeString(&usDriverName, L"\\Device\\ExampleDrv");
  RtlInitUnicodeString(&usDosDeviceName, L"\\DosDevices\\ExampleDrv");

  // Create the device driver
  NtStatus = IoCreateDevice(pDriverObject, sizeof(EXAMPLE_DEVICE_CONTEXT),
                            &usDriverName, FILE_DEVICE_UNKNOWN,
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

    // The unload function must always be included
    pDriverObject->DriverUnload = driverUnload;

    pExampleDeviceContext =
        (PEXAMPLE_DEVICE_CONTEXT)pDeviceObject->DeviceExtension;
    KeInitializeMutex(&pExampleDeviceContext->kmListMutex, 0);
    pExampleDeviceContext->pExampleList = NULL;

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