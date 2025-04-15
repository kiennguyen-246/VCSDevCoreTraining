#include "Entry.h"

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject,
                     PUNICODE_STRING pulRegistryPath) {
  UNREFERENCED_PARAMETER(pulRegistryPath);

  DbgPrint("DriverEntry called\n");

  NTSTATUS ntStatus = STATUS_SUCCESS;
  ntStatus =
      FltRegisterFilter(pDriverObject, &kFltRegistration, &driverData.pFilter);
  if (!NT_SUCCESS(ntStatus)) {
    DbgPrint("FltRegisterFilter failed: 0x%08X\n", ntStatus);
    return ntStatus;
  }

  PSECURITY_DESCRIPTOR psd;
  ntStatus = FltBuildDefaultSecurityDescriptor(&psd, FLT_PORT_ALL_ACCESS);
  if (ntStatus != STATUS_SUCCESS) {
    DbgPrint("Bulld security descriptor failed 0x%08x\n", ntStatus);
    FltUnregisterFilter(driverData.pFilter);
    return ntStatus;
  }

  UNICODE_STRING usMfltPortName;
  RtlInitUnicodeString(&usMfltPortName, kCOMPortName);

  OBJECT_ATTRIBUTES oa;
  InitializeObjectAttributes(&oa, &usMfltPortName,
                             OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL,
                             psd);
  ntStatus = FltCreateCommunicationPort(
      driverData.pFilter, &driverData.pServerPort, &oa, NULL, COMPortConnect,
      COMPortDisconnect, NULL, 1);
  FltFreeSecurityDescriptor(psd);
  if (ntStatus != STATUS_SUCCESS) {
    DbgPrint("Create COM port failed 0x%08x\n", ntStatus);
    FltUnregisterFilter(driverData.pFilter);
    return ntStatus;
  }

  ntStatus = FltStartFiltering(driverData.pFilter);
  if (ntStatus != STATUS_SUCCESS) {
    DbgPrint("Start filtering failed 0x%08x\n", ntStatus);
    FltCloseCommunicationPort(driverData.pServerPort);
    FltUnregisterFilter(driverData.pFilter);
    return ntStatus;
  }
  DbgPrint("Start filtering successful\n");

  driverData.bIsUnlocked = FALSE;

  return ntStatus;
}

NTSTATUS DriverExit(FLT_FILTER_UNLOAD_FLAGS fltUnloadFlags) {
  UNREFERENCED_PARAMETER(fltUnloadFlags);

  DbgPrint("DriverExit called\n");

  driverData.bIsComPortClosed = TRUE;
  FltCloseCommunicationPort(driverData.pServerPort);

  FltUnregisterFilter(driverData.pFilter);

  return STATUS_SUCCESS;
}

NTSTATUS DriverInstanceSetup(PCFLT_RELATED_OBJECTS pFltObjects,
                             FLT_INSTANCE_SETUP_FLAGS flags,
                             DEVICE_TYPE volumeDeviceType,
                             FLT_FILESYSTEM_TYPE volumeFilesystemType) {
  UNREFERENCED_PARAMETER(pFltObjects);
  UNREFERENCED_PARAMETER(flags);
  UNREFERENCED_PARAMETER(volumeDeviceType);
  UNREFERENCED_PARAMETER(volumeFilesystemType);

  DbgPrint("DriverInstanceSetup called\n");
  return STATUS_SUCCESS;
}

NTSTATUS DriverQueryTeardown(PCFLT_RELATED_OBJECTS pFltObjects,
                             FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags) {
  UNREFERENCED_PARAMETER(pFltObjects);
  UNREFERENCED_PARAMETER(Flags);

  DbgPrint("DriverQueryTeardown called\n");
  return STATUS_SUCCESS;
}