#include "COM.h"

NTSTATUS COMPortConnect(PFLT_PORT pClientPort, PVOID pServerPortCookie,
                        PVOID pConnectionContext, ULONG ulSizeOfContext,
                        PVOID* pConnectionCookie) {
  UNREFERENCED_PARAMETER(pServerPortCookie);
  UNREFERENCED_PARAMETER(pConnectionContext);
  UNREFERENCED_PARAMETER(ulSizeOfContext);
  UNREFERENCED_PARAMETER(pConnectionCookie);

  NTSTATUS ntStatus = STATUS_SUCCESS;

  DbgPrint("COMPortConnect called\n");
  FLT_ASSERT(driverData.pClientPort == NULL);
  driverData.pClientPort = pClientPort;

  driverData.bIsComPortClosed = FALSE;

  PFLT_GENERIC_WORKITEM pWorkItem = FltAllocateGenericWorkItem();
  if (pWorkItem == NULL) {
    DbgPrint("Allocate work item failed\n");
    return STATUS_INSUFFICIENT_RESOURCES;
  }
  ntStatus = FltQueueGenericWorkItem(pWorkItem, driverData.pFilter,
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

  DbgPrint("COMPortDisconnect called\n");
  driverData.bIsComPortClosed = TRUE;
  FltCloseClientPort(driverData.pFilter, &driverData.pClientPort);
  return STATUS_SUCCESS;
}

NTSTATUS COMPortMessageHandleRoutine(PFLT_GENERIC_WORKITEM pWorkItem,
                                     PVOID pFilterObject, PVOID pContext) {
  UNREFERENCED_PARAMETER(pFilterObject);
  UNREFERENCED_PARAMETER(pContext);

  // MFLT_SEND_MESSAGE sendMsg;
  NTSTATUS ntStatus;
  // LARGE_INTEGER liTimeOut = {0};

  ULONG ulPasswordBufferSize = 64;
  PCHAR pcSendBuffer = ExAllocatePool2(POOL_FLAG_NON_PAGED, 3, 'dnes');
  if (pcSendBuffer == NULL) {
    DbgPrint("Allocate send buffer failed\n");
    FltFreeGenericWorkItem(pWorkItem);
    return STATUS_INSUFFICIENT_RESOURCES;
  }
  pcSendBuffer[0] = 'o';
  pcSendBuffer[1] = 'k';
  pcSendBuffer[2] = 0;
  PUCHAR pbPasswordBuffer = ExAllocatePool2(
      POOL_FLAG_NON_PAGED, sizeof(FILTER_REPLY_HEADER) + ulPasswordBufferSize,
      'ssap');
  if (pbPasswordBuffer == NULL) {
    DbgPrint("Allocate password buffer failed\n");
    ExFreePool(pcSendBuffer);
    FltFreeGenericWorkItem(pWorkItem);
    return STATUS_INSUFFICIENT_RESOURCES;
  }
  RtlZeroMemory(pbPasswordBuffer,
                sizeof(FILTER_REPLY_HEADER) + ulPasswordBufferSize);
  ULONG ulPasswordBufferLength =
      sizeof(FILTER_REPLY_HEADER) + ulPasswordBufferSize;

  // liTimeOut.QuadPart = 1000;

  // Send "ok", expect 64 bits of Password
  if (driverData.pClientPort != NULL) {
    if (!driverData.bIsComPortClosed) {
      ntStatus = FltSendMessage(driverData.pFilter, &driverData.pClientPort,
                                pcSendBuffer, 3, pbPasswordBuffer,
                                &ulPasswordBufferLength, NULL);
      if (ntStatus != STATUS_SUCCESS) {
        DbgPrint("FltSendMessage failed 0x%08x\n", ntStatus);
      }
    }
  } else {
    DbgPrint("Client port not detected\n");
  }

  DbgPrint("Received password: %ws\n", pbPasswordBuffer);

  UNICODE_STRING usInputPassword, usRealPassword;
  RtlInitUnicodeString(&usInputPassword, (PWCHAR)pbPasswordBuffer);
  RtlInitUnicodeString(&usRealPassword, kPassword);
  if (!RtlCompareUnicodeString(&usInputPassword, &usRealPassword, FALSE)) {
    driverData.bIsUnlocked = TRUE;
    DbgPrint("Password is correct");
  } else {
    DbgPrint("Password is incorrect");
  }

  if (!driverData.bIsUnlocked) {
    pcSendBuffer[0] = 'k';
    pcSendBuffer[1] = 'o';
    pcSendBuffer[2] = 0;
  }

  // Send "ok" or "ko" based on the result
  if (driverData.pClientPort != NULL) {
    if (!driverData.bIsComPortClosed) {
      ntStatus = FltSendMessage(driverData.pFilter, &driverData.pClientPort,
                                pcSendBuffer, 3, NULL, 0, NULL);
      if (ntStatus != STATUS_SUCCESS) {
        DbgPrint("FltSendMessage failed 0x%08x\n", ntStatus);
      }
    }
  } else {
    DbgPrint("Client port not detected\n");
  }

  ExFreePool(pcSendBuffer);
  ExFreePool(pbPasswordBuffer);

  FltFreeGenericWorkItem(pWorkItem);

  return STATUS_SUCCESS;
}