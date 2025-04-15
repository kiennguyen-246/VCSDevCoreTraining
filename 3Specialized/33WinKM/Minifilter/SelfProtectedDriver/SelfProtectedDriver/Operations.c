#include "Operations.h"

FLT_PREOP_CALLBACK_STATUS CreateFilePreops(PFLT_CALLBACK_DATA pData,
                                           PCFLT_RELATED_OBJECTS pFltObjects,
                                           PVOID* pCompletionContext) {
  UNREFERENCED_PARAMETER(pFltObjects);

  // DbgPrint("CreateFilePreops called\n");

  pCompletionContext = NULL;

  PFILE_OBJECT pTargetFileObject = pData->Iopb->TargetFileObject;
  ACCESS_MASK desiredAccess =
      pData->Iopb->Parameters.Create.SecurityContext->DesiredAccess;

  // DbgPrint("CreateFile invoked at %ws, access mask: 0x%08x\n",
  //          pTargetFileObject->FileName.Buffer, desiredAccess);

  if (driverData.bIsUnlocked) {
    return FLT_PREOP_SUCCESS_NO_CALLBACK;
  }

  if (!pTargetFileObject->FileName.Length) {
    return FLT_PREOP_SUCCESS_NO_CALLBACK;
  }

  for (UINT32 i = 0; i < kMaxTargetPath; ++i) {
    UNICODE_STRING usTargetPath;
    RtlInitUnicodeString(&usTargetPath, kTargetPaths[i]);
    if (!RtlCompareUnicodeString(&usTargetPath, &(pTargetFileObject->FileName),
                                 TRUE)) {
      if ((desiredAccess & ~kAllowedAccess) != 0) {
        DbgPrint("Successfully denied access\n");
        pData->IoStatus.Status = STATUS_ACCESS_DENIED;
        pData->IoStatus.Information = 0;
        return FLT_PREOP_COMPLETE;
      }
    }
  }

  return FLT_PREOP_SUCCESS_NO_CALLBACK;
}