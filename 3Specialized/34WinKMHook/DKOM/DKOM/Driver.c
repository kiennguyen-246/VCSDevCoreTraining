#include "MFLTData.h"

#define DbgPrint(x, ...) DbgPrint(" [DKOM]" x, __VA_ARGS__)
#define MFLT_COM_PORT_NAME L"\\DKOMCOM"

MFLT_DATA mfltData;

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);
NTSTATUS DriverExit(FLT_FILTER_UNLOAD_FLAGS fltUnloadFlags);
NTSTATUS DriverQueryTeardown(PCFLT_RELATED_OBJECTS pFltObjects,
                             FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags);
NTSTATUS COMPortConnect(PFLT_PORT pClientPort, PVOID pServerPortCookie,
                        PVOID pConnectionContext, ULONG uiSizeOfContext,
                        PVOID* pConnectionCookie);
NTSTATUS COMPortDisconnect(PVOID pConnectionCookie);
NTSTATUS COMPortMessageHandleRoutine(PFLT_GENERIC_WORKITEM pWorkItem,
                                     PVOID pFilterObject, PVOID pContext);

CONST FLT_REGISTRATION kFltRegistration = {sizeof(FLT_REGISTRATION),
                                           FLT_REGISTRATION_VERSION,
                                           FLTFL_REGISTRATION_SUPPORT_NPFS_MSFS,
                                           NULL,
                                           NULL,
                                           DriverExit,
                                           NULL,
                                           DriverQueryTeardown,
                                           NULL,
                                           NULL,
                                           NULL,
                                           NULL,
                                           NULL,
                                           NULL,
                                           NULL};

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject,
                     PUNICODE_STRING puiRegistryPath) {
  UNREFERENCED_PARAMETER(puiRegistryPath);

  DbgPrint("DriverEntry called\n");

  NTSTATUS ntStatus = STATUS_SUCCESS;
  ntStatus =
      FltRegisterFilter(pDriverObject, &kFltRegistration, &mfltData.pFilter);
  if (!NT_SUCCESS(ntStatus)) {
    DbgPrint("FltRegisterFilter failed: 0x%08X\n", ntStatus);
    return ntStatus;
  }

  PSECURITY_DESCRIPTOR psd;
  ntStatus = FltBuildDefaultSecurityDescriptor(&psd, FLT_PORT_ALL_ACCESS);
  if (ntStatus != STATUS_SUCCESS) {
    DbgPrint("Build security descriptor failed 0x%08x\n", ntStatus);
    FltUnregisterFilter(mfltData.pFilter);
    return ntStatus;
  }

  UNICODE_STRING usMfltPortName;
  RtlInitUnicodeString(&usMfltPortName, MFLT_COM_PORT_NAME);

  OBJECT_ATTRIBUTES oa;
  InitializeObjectAttributes(&oa, &usMfltPortName,
                             OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL,
                             psd);
  ntStatus = FltCreateCommunicationPort(mfltData.pFilter, &mfltData.pServerPort,
                                        &oa, NULL, COMPortConnect,
                                        COMPortDisconnect, NULL, 1);
  FltFreeSecurityDescriptor(psd);
  if (ntStatus != STATUS_SUCCESS) {
    DbgPrint("Create COM port failed 0x%08x\n", ntStatus);
    FltUnregisterFilter(mfltData.pFilter);
    return ntStatus;
  }

  ntStatus = FltStartFiltering(mfltData.pFilter);
  if (ntStatus != STATUS_SUCCESS) {
    DbgPrint("Start filtering failed 0x%08x\n", ntStatus);
    FltCloseCommunicationPort(mfltData.pServerPort);
    FltUnregisterFilter(mfltData.pFilter);
    return ntStatus;
  }
  DbgPrint("Start filtering successful\n");

  return ntStatus;
}

NTSTATUS DriverExit(FLT_FILTER_UNLOAD_FLAGS fltUnloadFlags) {
  UNREFERENCED_PARAMETER(fltUnloadFlags);

  DbgPrint("DriverExit called\n");

  FltCloseCommunicationPort(mfltData.pServerPort);

  FltUnregisterFilter(mfltData.pFilter);

  return STATUS_SUCCESS;
}

NTSTATUS DriverQueryTeardown(PCFLT_RELATED_OBJECTS pFltObjects,
                             FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags) {
  UNREFERENCED_PARAMETER(pFltObjects);
  UNREFERENCED_PARAMETER(Flags);

  return STATUS_SUCCESS;
}

NTSTATUS COMPortConnect(PFLT_PORT pClientPort, PVOID pServerPortCookie,
                        PVOID pConnectionContext, ULONG uiSizeOfContext,
                        PVOID* pConnectionCookie) {
  UNREFERENCED_PARAMETER(pServerPortCookie);
  UNREFERENCED_PARAMETER(pConnectionContext);
  UNREFERENCED_PARAMETER(uiSizeOfContext);
  UNREFERENCED_PARAMETER(pConnectionCookie);

  DbgPrint("mfltComConnect called\n");
  FLT_ASSERT(mfltData.pClientPort == NULL);
  mfltData.pClientPort = pClientPort;
  return STATUS_SUCCESS;
}

NTSTATUS COMPortDisconnect(PVOID pConnectionCookie) {
  UNREFERENCED_PARAMETER(pConnectionCookie);

  DbgPrint("mfltComDisconnect called\n");
  mfltData.bIsComPortClosed = TRUE;
  FltCloseClientPort(mfltData.pFilter, &mfltData.pClientPort);
  return STATUS_SUCCESS;
}

NTSTATUS COMPortMessageHandleRoutine(PFLT_GENERIC_WORKITEM pWorkItem,
    PVOID pFilterObject, PVOID pContext) {
  // MFLT_SEND_MESSAGE sendMsg;
  NTSTATUS ntStatus;
  LARGE_INTEGER liTimeOut = {0};

  if (mfltData.pClientPort != NULL) {

    if (!mfltData.bIsComPortClosed) {
      ntStatus =
          FltSendMessage(mfltData.pFilter, &mfltData.pClientPort, pContext,
                         sizeof(MFLT_EVENT_RECORD), NULL, 0, NULL);
      if (ntStatus != STATUS_SUCCESS) {
        DbgPrint("Send event record failed 0x%08x\n", ntStatus);
      }
    }
  }
  ExFreePool(pContext);
  FltFreeGenericWorkItem(pWorkItem);
}