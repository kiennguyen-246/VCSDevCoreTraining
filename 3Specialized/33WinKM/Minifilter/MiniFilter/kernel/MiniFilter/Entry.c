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

  /*mfltData.uiEventRecordCount = 0;
  mfltData.pEventRecordBuffer = ExAllocatePool2(
      POOL_FLAG_NON_PAGED | POOL_FLAG_RAISE_ON_FAILURE,
      sizeof(MFLT_EVENT_RECORD) * MAX_BUFFERED_EVENT_COUNT,
      "BREM");
  if (mfltData.pEventRecordBuffer == NULL) {
    DbgPrint("Cannot allocate memory for the event record buffer");
    return STATUS_BUFFER_TOO_SMALL;
  }*/

  ntStatus = PsSetCreateProcessNotifyRoutineEx(mfltCreateProcessNotify, FALSE);
  if (ntStatus != STATUS_SUCCESS) {
    DbgPrint("Start CreateProcess notify routine failed 0x%08x\n", ntStatus);
  } else {
    DbgPrint("Start CreateProcess notify routine successfully\n", ntStatus);
  }

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
  // DbgPrint("mfltPreCreate called\n");

  // DbgBreakPoint();

  // NTSTATUS ntStatus = STATUS_SUCCESS;
  // UNICODE_STRING usVolumeName, usComMsg, usComMsgPref, usComMsgSuf;
  // WCHAR pwcInitString[MAX_BUFFER_SIZE];
  // ULONG ulVolumeNameBufferSize;
  // PMFLT_EVENT_RECORD pEventRecord;
  // LARGE_INTEGER liSystemTime = {0};
  // LARGE_INTEGER liSystemLocalTime = {0};
  // PFLT_GENERIC_WORKITEM pWorkItem = NULL;

  // if (pFltObj->FileObject->FileName.Buffer == NULL) {
  //   return FLT_PREOP_SUCCESS_WITH_CALLBACK;
  // }

  //// DbgBreakPoint();
  // pEventRecord = (PMFLT_EVENT_RECORD)ExAllocatePool2(
  //     POOL_FLAG_NON_PAGED, sizeof(MFLT_EVENT_RECORD), 'REFM');
  // RtlZeroMemory(pEventRecord, sizeof(MFLT_EVENT_RECORD));

  // KeQuerySystemTime(&liSystemTime);
  // ExSystemTimeToLocalTime(&liSystemTime, &liSystemLocalTime);
  // pEventRecord->uliSysTime.QuadPart = liSystemLocalTime.QuadPart;

  // wcscat(pEventRecord->objInfo.fileInfo.pwcFileName,
  //        pFltObj->FileObject->FileName.Buffer);
  // pEventRecord->objInfo.fileInfo.uiFileNameLength =
  //     pFltObj->FileObject->FileName.Length;

  // for (int i = 0; i < UM_MAX_PATH - 1; i++) {
  //   pwcInitString[i] = L' ';
  // }
  // RtlInitUnicodeString(&usVolumeName, pwcInitString);
  //// DbgBreakPoint();
  // ntStatus =
  //     FltGetVolumeName(pFltObj->Volume, &usVolumeName,
  //     &ulVolumeNameBufferSize);
  //// DbgBreakPoint();
  // if (ntStatus != STATUS_SUCCESS) {
  //   DbgPrint("GetVolumeName failed 0x%08x", ntStatus);
  // }
  // wcscat(pEventRecord->objInfo.fileInfo.pwcVolumeName, usVolumeName.Buffer);
  // pEventRecord->objInfo.fileInfo.uiVolumeNameLength = usVolumeName.Length;

  // switch (pCallbackData->Iopb->MajorFunction) {
  //   case IRP_MJ_CREATE:
  //     DbgPrint("File object opened: %wZ%wZ\n", usVolumeName,
  //              pFltObj->FileObject->FileName);

  //    DbgPrint("Create options:");
  //    for (int i = 0; i < 24; i++) {
  //      if ((pCallbackData->Iopb->Parameters.Create.Options & (1 << i)) != 0)
  //      {
  //        DbgPrint("0x%08x, ", (1 << i));
  //      }
  //    }
  //    DbgPrint("\n");

  //    DbgPrint("Disposition: ");
  //    switch (pCallbackData->Iopb->Parameters.Create.Options >> 24) {
  //      case FILE_SUPERSEDE:
  //        DbgPrint("FILE_SUPERSEDE\n");
  //        break;
  //      case FILE_OPEN:
  //        DbgPrint("FILE_OPEN\n");
  //        break;
  //      case FILE_CREATE:
  //        DbgPrint("FILE_CREATE\n");
  //        break;
  //      case FILE_OPEN_IF:
  //        DbgPrint("FILE_OPEN_IF\n");
  //        break;
  //      case FILE_OVERWRITE:
  //        DbgPrint("FILE_OVERWRITE\n");
  //        break;
  //      case FILE_OVERWRITE_IF:
  //        DbgPrint("FILE_OVERWRITE_IF\n");
  //        break;
  //      default:
  //        break;
  //    }

  //    DbgPrint("Status: 0x%08x\n", pCallbackData->IoStatus.Status);

  //    pEventRecord->eventType = MFLT_OPEN;
  //    pEventRecord->objInfo.fileInfo.bIsDirectory =
  //        ((pCallbackData->Iopb->Parameters.Create.Options &
  //          FILE_DIRECTORY_FILE) != 0);
  //    break;
  //  case IRP_MJ_CLOSE:
  //    // DbgPrint("File object closed: %wZ%wZ\n", usVolumeName,
  //    // pFltObj->FileObject->FileName);
  //    pEventRecord->eventType = MFLT_CLOSE;
  //    break;
  //  case IRP_MJ_WRITE:
  //    // DbgPrint("File object written: %wZ%wZ\n", usVolumeName,
  //    // pFltObj->FileObject->FileName);
  //    pEventRecord->eventType = MFLT_WRITE;
  //    break;
  //  default:
  //    break;
  //}

  // pWorkItem = FltAllocateGenericWorkItem();
  // FltQueueGenericWorkItem(pWorkItem, mfltData.pFilter,
  //                         mfltSendMessageWorkItemRoutine, DelayedWorkQueue,
  //                         pEventRecord);

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
  NTSTATUS ntStatus = STATUS_SUCCESS;
  UNICODE_STRING usVolumeName, usComMsg, usComMsgPref, usComMsgSuf;
  WCHAR pwcInitString[MAX_BUFFER_SIZE];
  ULONG ulVolumeNameBufferSize;
  PMFLT_EVENT_RECORD pEventRecord;
  LARGE_INTEGER liSystemTime = {0};
  LARGE_INTEGER liSystemLocalTime = {0};
  PFLT_GENERIC_WORKITEM pWorkItem = NULL;

  if (pFltObj->FileObject->FileName.Buffer == NULL) {
    return FLT_PREOP_SUCCESS_WITH_CALLBACK;
  }

  // DbgBreakPoint();
  pEventRecord = (PMFLT_EVENT_RECORD)ExAllocatePool2(
      POOL_FLAG_NON_PAGED, sizeof(MFLT_EVENT_RECORD), 'REFM');
  RtlZeroMemory(pEventRecord, sizeof(MFLT_EVENT_RECORD));

  KeQuerySystemTime(&liSystemTime);
  ExSystemTimeToLocalTime(&liSystemTime, &liSystemLocalTime);
  pEventRecord->uliSysTime.QuadPart = liSystemLocalTime.QuadPart;

  // wcscat(pEventRecord->objInfo.fileInfo.pwcFileName,
  //        pFltObj->FileObject->FileName.Buffer);
  RtlCopyMemory(pEventRecord->objInfo.fileInfo.pwcFileName,
                pFltObj->FileObject->FileName.Buffer,
                pFltObj->FileObject->FileName.Length);
  pEventRecord->objInfo.fileInfo.uiFileNameLength =
      pFltObj->FileObject->FileName.Length;

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
  wcscat(pEventRecord->objInfo.fileInfo.pwcVolumeName, usVolumeName.Buffer);
  RtlCopyMemory(pEventRecord->objInfo.fileInfo.pwcVolumeName,
                usVolumeName.Buffer, usVolumeName.Length);
  pEventRecord->objInfo.fileInfo.uiVolumeNameLength = usVolumeName.Length;

  pEventRecord->objInfo.fileInfo.iOperationStatus =
      pCallbackData->IoStatus.Status;

  switch (pCallbackData->Iopb->MajorFunction) {
    case IRP_MJ_CREATE:
      // DbgPrint("File object opened: %wZ%wZ\n", usVolumeName,
      //          pFltObj->FileObject->FileName);
      pEventRecord->eventType = MFLT_OPEN;

      // DbgPrint("Create options:");
      // for (int i = 0; i < 24; i++) {
      //   if ((pCallbackData->Iopb->Parameters.Create.Options & (1 << i)) != 0)
      //   {
      //     DbgPrint("0x%08x, ", (1 << i));
      //   }
      // }
      // DbgPrint("\n");
      pEventRecord->objInfo.fileInfo.bIsDirectory =
          ((pCallbackData->Iopb->Parameters.Create.Options &
            FILE_DIRECTORY_FILE) != 0);
      if ((pCallbackData->Iopb->Parameters.Create.Options &
           FILE_DELETE_ON_CLOSE) != 0) {
        pEventRecord->eventType = MFLT_DELETE;
      }

      // DbgPrint("Disposition: ");
      switch (pCallbackData->Iopb->Parameters.Create.Options >> 24) {
        case FILE_SUPERSEDE:
        case FILE_OVERWRITE_IF:
          pEventRecord->eventType = MFLT_CREATE;
          pEventRecord->objInfo.fileInfo.bIsOverwritten = TRUE;
          break;
        case FILE_CREATE:
        case FILE_OPEN_IF:
          pEventRecord->eventType = MFLT_CREATE;
          pEventRecord->objInfo.fileInfo.bIsOverwritten = FALSE;
          break;
        case FILE_OPEN:
          pEventRecord->objInfo.fileInfo.bIsOverwritten = FALSE;
          break;
        case FILE_OVERWRITE:
          pEventRecord->objInfo.fileInfo.bIsOverwritten = TRUE;
          break;
        default:
          break;
      }

      // DbgPrint("Status: 0x%08x\n", pCallbackData->IoStatus.Status);

      break;
    case IRP_MJ_CLOSE:
      // DbgPrint("File object closed: %wZ%wZ\n", usVolumeName,
      // pFltObj->FileObject->FileName);
      pEventRecord->eventType = MFLT_CLOSE;
      break;
    case IRP_MJ_WRITE:
      // DbgPrint("File object written: %wZ%wZ\n", usVolumeName,
      // pFltObj->FileObject->FileName);
      pEventRecord->eventType = MFLT_WRITE;
      break;
    default:
      break;
  }

  // DbgPrint("Status: 0x%08x\n", pCallbackData->IoStatus.Status);
  // ExFreePool(pEventRecord);

  pWorkItem = FltAllocateGenericWorkItem();
  FltQueueGenericWorkItem(pWorkItem, mfltData.pFilter,
                          mfltSendMessageWorkItemRoutine, DelayedWorkQueue,
                          pEventRecord);

  return FLT_POSTOP_FINISHED_PROCESSING;
}

VOID mfltCreateProcessNotify(PEPROCESS pProcess, HANDLE hPid,
                             PPS_CREATE_NOTIFY_INFO pCreateInfo) {
  PMFLT_EVENT_RECORD pEventRecord;
  LARGE_INTEGER liSystemTime = {0};
  LARGE_INTEGER liSystemLocalTime = {0};
  PFLT_GENERIC_WORKITEM pWorkItem = NULL;
  WCHAR pwcTruncated[] = L"... (truncated)";

  pEventRecord = (PMFLT_EVENT_RECORD)ExAllocatePool2(
      POOL_FLAG_NON_PAGED, sizeof(MFLT_EVENT_RECORD), 'REFM');
  RtlZeroMemory(pEventRecord, sizeof(MFLT_EVENT_RECORD));

  // DbgBreakPoint();

  KeQuerySystemTime(&liSystemTime);
  ExSystemTimeToLocalTime(&liSystemTime, &liSystemLocalTime);
  pEventRecord->uliSysTime.QuadPart = liSystemLocalTime.QuadPart;
  if (pCreateInfo != NULL) {
    // DbgPrint("Process PID = %d created from parent process PID = %d\n", hPid,
    // pCreateInfo->ParentProcessId);
    pEventRecord->eventType = MFLT_PROCESS_CREATE;
    pEventRecord->objInfo.procInfo.uiPID = (ULONG)hPid;
    pEventRecord->objInfo.procInfo.uiParentPID =
        (ULONG)pCreateInfo->ParentProcessId;
    if (pCreateInfo->ImageFileName->Buffer != NULL) {
      //DbgBreakPoint();
      RtlCopyMemory(pEventRecord->objInfo.procInfo.pwcImageName,
                    pCreateInfo->ImageFileName->Buffer,
                    pCreateInfo->ImageFileName->Length);
      pEventRecord->objInfo.procInfo.uiImageNameLength =
          pCreateInfo->ImageFileName->Length;
    }
    // DbgBreakPoint();
    if (pCreateInfo->CommandLine->Buffer != NULL) {
      if (pCreateInfo->CommandLine->Length <= UM_MAX_PATH) {
        RtlCopyMemory(pEventRecord->objInfo.procInfo.pwcCommandLine,
                      pCreateInfo->CommandLine->Buffer,
                      pCreateInfo->CommandLine->Length);
        /*wcscat(pEventRecord->objInfo.procInfo.pwcCommandLine,
               pCreateInfo->CommandLine->Buffer)*/
        ;
        pEventRecord->objInfo.procInfo.uiCommandLineLength =
            pCreateInfo->CommandLine->Length;
      } else {
        RtlCopyMemory(pEventRecord->objInfo.procInfo.pwcCommandLine,
                      pCreateInfo->CommandLine->Buffer,
                      (UM_MAX_PATH - 1 - wcslen(pwcTruncated)));
        wcscat(pEventRecord->objInfo.procInfo.pwcCommandLine, pwcTruncated);
        pEventRecord->objInfo.procInfo.uiCommandLineLength = wcslen(pEventRecord->objInfo.procInfo.pwcCommandLine);
      }
    }

  } else {
    NTSTATUS ntsExitCode = PsGetProcessExitStatus(pProcess);
    // DbgPrint("Process PID = %d exited with exitcode %d (0x%08x).\n", hPid,
    //          ntsExitCode, ntsExitCode);
    //  DbgPrint("Process PID = %d exited.\n", hPid);
    pEventRecord->eventType = MFLT_PROCESS_TERMINATE;
    pEventRecord->objInfo.procInfo.uiPID = (ULONG)hPid;
    pEventRecord->objInfo.procInfo.iExitcode = (ULONG)ntsExitCode;
  }

  pWorkItem = FltAllocateGenericWorkItem();
  FltQueueGenericWorkItem(pWorkItem, mfltData.pFilter,
                          mfltSendMessageWorkItemRoutine, DelayedWorkQueue,
                          pEventRecord);
}

VOID mfltSendMessageWorkItemRoutine(PFLT_GENERIC_WORKITEM pWorkItem,
                                    PVOID pFilterObject, PVOID pContext) {
  // MFLT_SEND_MESSAGE sendMsg;
  NTSTATUS ntStatus;
  LARGE_INTEGER liTimeOut = {0};

  // DbgBreakPoint();

  if (mfltData.pClientPort != NULL) {
    // RtlZeroMemory(&sendMsg, sizeof(sendMsg));
    // liTimeOut.QuadPart = MAX_TIMEOUT;

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