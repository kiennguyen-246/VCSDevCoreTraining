#include "Entry.h"

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject,
                     PUNICODE_STRING pusRegistryPath) {
  DbgPrint("DriverEntry called\n");
  NTSTATUS ntStatus = STATUS_SUCCESS;
  PSECURITY_DESCRIPTOR psd;
  OBJECT_ATTRIBUTES oa;
  UNICODE_STRING usMfltPortName;

  mfltData.pDriverObject = pDriverObject;

  // Register the filter
  ntStatus =
      FltRegisterFilter(pDriverObject, &fltRegistration, &mfltData.pFilter);
  if (ntStatus != STATUS_SUCCESS) {
    DbgPrint("Register filter failed 0x%08x\n", ntStatus);
    return ntStatus;
  }
  // DbgPrint("Register filter successful\n");

  // DbgBreakPoint();

  // Create a communication port
  ntStatus = FltBuildDefaultSecurityDescriptor(&psd, FLT_PORT_ALL_ACCESS);
  if (ntStatus != STATUS_SUCCESS) {
    DbgPrint("Build security descriptor failed 0x%08x\n", ntStatus);
    FltUnregisterFilter(mfltData.pFilter);
    return ntStatus;
  }

  RtlInitUnicodeString(&usMfltPortName, MFLT_COM_PORT_NAME);

  InitializeObjectAttributes(&oa, &usMfltPortName,
                             OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL,
                             psd);
  ntStatus = FltCreateCommunicationPort(mfltData.pFilter, &mfltData.pServerPort,
                                        &oa, NULL, mfltComConnect,
                                        mfltComDisconnect, NULL, 1);
  FltFreeSecurityDescriptor(psd);
  if (ntStatus != STATUS_SUCCESS) {
    DbgPrint("Create COM port failed 0x%08x\n", ntStatus);
    FltUnregisterFilter(mfltData.pFilter);
    return ntStatus;
  }

  ntStatus = PsSetCreateProcessNotifyRoutineEx(mfltCreateProcessNotify, FALSE);
  if (ntStatus != STATUS_SUCCESS) {
    DbgPrint("Start CreateProcess notify routine failed 0x%08x\n", ntStatus);
  } else {
    DbgPrint("Start CreateProcess notify routine successfully\n", ntStatus);
  }

  mfltData.pWorkItem = FltAllocateGenericWorkItem();
  InitializeListHead(&mfltData.eventRecordList.list);

  // Start filtering (Must unregister immediately if fail)
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

NTSTATUS DriverUnload(FLT_FILTER_UNLOAD_FLAGS fltUnloadFlags) {
  DbgPrint("DriverUnload called\n");

  PsSetCreateProcessNotifyRoutineEx(mfltCreateProcessNotify, TRUE);

  FltCloseCommunicationPort(mfltData.pServerPort);

  FltFreeGenericWorkItem(mfltData.pWorkItem);

  // Unregister filter
  FltUnregisterFilter(mfltData.pFilter);

  return STATUS_SUCCESS;
}

NTSTATUS DriverQueryTeardown(PCFLT_RELATED_OBJECTS FltObjects,
                             FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags) {
  DbgPrint("DriverQueryTeardown called\n");
  return STATUS_SUCCESS;
}

NTSTATUS mfltComConnect(PFLT_PORT pClientPort, PVOID pServerPortCookie,
                        PVOID pConnectionContext, ULONG uiSizeOfContext,
                        PVOID* pConnectionCookie) {
  DbgPrint("mfltComConnect called\n");
  FLT_ASSERT(mfltData.pClientPort == NULL);
  mfltData.pClientPort = pClientPort;
  return STATUS_SUCCESS;
}

NTSTATUS mfltComDisconnect(PVOID pConnectionCookie) {
  DbgPrint("mfltComDisconnect called\n");
  mfltData.bIsComPortClosed = TRUE;
  FltCloseClientPort(mfltData.pFilter, &mfltData.pClientPort);
  return STATUS_SUCCESS;
}

// NTSTATUS comMessage(PVOID pConnectionCookie, PVOID pInputBuffer,
//                     ULONG uiInputBufferSize, PVOID pcOutputBuffer,
//                     ULONG uiOutputBufferSize,
//                     PULONG puiReturnOutputBufferLength) {
//   NTSTATUS ntStatus = STATUS_SUCCESS;
//   if (pInputBuffer != NULL) {
//     if (pcOutputBuffer != NULL && uiOutputBufferSize != 0) {
//       PLIST_ENTRY pleCurNode;
//       ULONG ulBytesWritten;
//       NTSTATUS ntStatus = STATUS_SUCCESS;
//       KIRQL oldIrql;
//       BOOLEAN bIsRecordsAvailable = FALSE;
//
//       KeAcquireSpinLock(&mfltData.kslOutputBufferLock, &oldIrql);
//
//       while (!IsListEmpty(&mfltData.leOutputBufferList) &&
//       (uiOutputBufferSize > 0)) {
//         bIsRecordsAvailable = TRUE;
//
//         pleCurNode = RemoveHeadList(&mfltData.leOutputBufferList);
//       }
//
//     } else {
//       ntStatus = STATUS_INVALID_PARAMETER;
//     }
//   }
// }

VOID mfltContextCleanup(PFLT_CONTEXT pFltContext,
                        FLT_CONTEXT_TYPE fltContextType) {}

FLT_PREOP_CALLBACK_STATUS mfltPreOp(PFLT_CALLBACK_DATA pCallbackData,
                                    PCFLT_RELATED_OBJECTS pFltObj,
                                    PVOID* pCompletionContext) {
  // DbgBreakPoint();
  // DbgPrint("mfltPreCreate called\n");

  // DbgBreakPoint();

  NTSTATUS ntStatus = STATUS_SUCCESS;
  UNICODE_STRING usVolumeName, usComMsg, usComMsgPref, usComMsgSuf;
  WCHAR pwcInitString[MAX_BUFFER_SIZE];
  ULONG ulVolumeNameBufferSize;
  MFLT_EVENT_RECORD_LIST eventRecordList;
  LARGE_INTEGER liSystemTime = {0};
  LARGE_INTEGER liSystemLocalTime = {0};

  RtlZeroMemory(&eventRecordList, sizeof(eventRecordList));
  InsertTailList(&mfltData.eventRecordList, &eventRecordList.list);

  KeQuerySystemTime(&liSystemTime);
  ExSystemTimeToLocalTime(&liSystemTime, &liSystemLocalTime);
  eventRecordList.eventRecord.uliSysTime.QuadPart = liSystemLocalTime.QuadPart;

  if (pFltObj->FileObject->FileName.Buffer == NULL) {
    return FLT_PREOP_SUCCESS_WITH_CALLBACK;
  }

  wcscat(eventRecordList.eventRecord.objInfo.fileInfo.pwcFileName,
         pFltObj->FileObject->FileName.Buffer);

  for (int i = 0; i < UM_MAX_PATH - 1; i++) {
    pwcInitString[i] = L' ';
  }
  RtlInitUnicodeString(&usVolumeName, pwcInitString);
  // DbgBreakPoint();
  ntStatus =
      FltGetVolumeName(pFltObj->Volume, &usVolumeName, &ulVolumeNameBufferSize);
  // DbgBreakPoint();
  if (ntStatus != STATUS_SUCCESS) {
    DbgPrint("GetVolumeName failed 0x%08x", ntStatus);
  }
  wcscat(eventRecordList.eventRecord.objInfo.fileInfo.pwcVolumeName,
         usVolumeName.Buffer);

  switch (pCallbackData->Iopb->MajorFunction) {
    case IRP_MJ_CREATE:
      DbgPrint("File object opened: %wZ%wZ\n", usVolumeName,
               pFltObj->FileObject->FileName);
      eventRecordList.eventRecord.eventType = MFLT_OPEN;
      eventRecordList.eventRecord.objInfo.fileInfo.bIsDirectory =
          ((pCallbackData->Iopb->OperationFlags & FILE_DIRECTORY_FILE) != 0);
      break;
    case IRP_MJ_CLOSE:
      DbgPrint("File object closed: %wZ%wZ\n", usVolumeName,
               pFltObj->FileObject->FileName);
      eventRecordList.eventRecord.eventType = MFLT_CLOSE;
      break;
    case IRP_MJ_WRITE:
      DbgPrint("File object written: %wZ%wZ\n", usVolumeName,
               pFltObj->FileObject->FileName);
      eventRecordList.eventRecord.eventType = MFLT_WRITE;
      break;
    default:
      break;
  }

  DbgBreakPoint();

  ntStatus = FltQueueGenericWorkItem(
      mfltData.pWorkItem, mfltData.pFilter, mfltComSendMessageWorkItemRoutine,
      DelayedWorkQueue, NULL);
  if (ntStatus != STATUS_SUCCESS) {
    DbgPrint("Queuing send message work item failed 0x%08x", ntStatus);
  }

  // DbgPrint("Open file: %wZ\n", pFltObj->FileObject->FileName);

  return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

FLT_POSTOP_CALLBACK_STATUS mfltPostOp(
    PFLT_CALLBACK_DATA pCallbackData, PCFLT_RELATED_OBJECTS pFltObj,
    PVOID* pCompletionContext, FLT_POST_OPERATION_FLAGS postOperationFlags) {
  // DbgBreakPoint();
  // DbgPrint("mfltPostCreate called\n");

  // if (pCallbackData->Iopb->MajorFunction == IRP_MJ_CREATE) {
  //   DbgPrint("%d\n",
  //            (pCallbackData->Iopb->OperationFlags & FILE_DIRECTORY_FILE) !=
  //            0);
  // }

  return FLT_POSTOP_FINISHED_PROCESSING;
}

VOID mfltCreateProcessNotify(PEPROCESS pProcess, HANDLE hPid,
                             PPS_CREATE_NOTIFY_INFO pCreateInfo) {
  DbgBreakPoint();

  NTSTATUS ntStatus = STATUS_SUCCESS;
  MFLT_EVENT_RECORD eventRecord;
  LARGE_INTEGER liSystemTime = {0};
  LARGE_INTEGER liSystemLocalTime = {0};

  RtlZeroMemory(&eventRecord, sizeof(eventRecord));

  KeQuerySystemTime(&liSystemTime);
  ExSystemTimeToLocalTime(&liSystemTime, &liSystemLocalTime);
  eventRecord.uliSysTime.QuadPart = liSystemLocalTime.QuadPart;
  if (pCreateInfo != NULL) {
    DbgPrint("Process PID = %d created from parent process PID = %d\n", hPid,
             pCreateInfo->ParentProcessId);
    eventRecord.eventType = MFLT_PROCESS_CREATE;
    eventRecord.objInfo.procInfo.uiPID = (ULONG)hPid;
    eventRecord.objInfo.procInfo.uiParentPID =
        (ULONG)pCreateInfo->ParentProcessId;
    if (pCreateInfo->ImageFileName->Buffer != NULL) {
      wcscat(eventRecord.objInfo.procInfo.pwcImageName,
             pCreateInfo->ImageFileName->Buffer);
    }
    if (pCreateInfo->CommandLine->Buffer != NULL) {
      wcscat(eventRecord.objInfo.procInfo.pwcCommandLine,
             pCreateInfo->CommandLine->Buffer);
    }

  } else {
    NTSTATUS ntsExitCode = PsGetProcessExitStatus(pProcess);
    DbgPrint("Process PID = %d exited with exitcode %d (0x%08x).\n", hPid,
             ntsExitCode, ntsExitCode);
    eventRecord.eventType = MFLT_PROCESS_TERMINATE;
    eventRecord.objInfo.procInfo.uiPID = (ULONG)hPid;
    eventRecord.objInfo.procInfo.iExitcode = (ULONG)ntsExitCode;
  }

  // ntStatus = FltQueueGenericWorkItem(mfltData.pWorkItem, mfltData.pFilter,
  //                                    mfltComSendMessageWorkItemRoutine,
  //                                    DelayedWorkQueue, &eventRecord);
  // if (ntStatus != STATUS_SUCCESS) {
  //   DbgPrint("Queuing send message work item failed 0x%08x", ntStatus);
  // }
}

VOID mfltComSendMessageWorkItemRoutine(PFLT_GENERIC_WORKITEM pWorkItem,
                                       PVOID pFltObject, PVOID pContext) {
  DbgBreakPoint();

  PLIST_ENTRY pCurrentListEntry = RemoveHeadList(&mfltData.eventRecordList.list);
  if (pCurrentListEntry == NULL) {
    return;
  }
  PMFLT_EVENT_RECORD pEventRecord =
      &CONTAINING_RECORD(pCurrentListEntry, MFLT_EVENT_RECORD_LIST, list)
          ->eventRecord;
  if (mfltData.pClientPort != NULL) {
    LARGE_INTEGER liTimeOut;
    liTimeOut.QuadPart = MAX_TIMEOUT;
    if (!mfltData.bIsComPortClosed) {
      NTSTATUS ntStatus =
          FltSendMessage(mfltData.pFilter, &mfltData.pClientPort,
                         pEventRecord,
                         sizeof(MFLT_EVENT_RECORD), NULL, 0, NULL);
      if (ntStatus != STATUS_SUCCESS) {
        DbgPrint("Send event record failed 0x%08x\n", ntStatus);
        // DbgBreakPoint();
      }
    }
  }
  pEventRecord = NULL;
}