#include "include.h"
#include "public.h"

NTSTATUS handleSampleIoctlBufferedIo(PIRP pIrp,
                                     PIO_STACK_LOCATION pIoStackLocation,
                                     UINT32* pwDataWritten);
NTSTATUS createPipeContext(PEXAMPLE_DEVICE_CONTEXT pExampleDeviceContext,
                           PFILE_OBJECT pFileObject);
NTSTATUS releasePipeContext(PEXAMPLE_DEVICE_CONTEXT pExampleDeviceContext,
                            PFILE_OBJECT pFileObject);
BOOLEAN writePipeContext(PEXAMPLE_LIST pExampleList, PCHAR pcData,
                         UINT32 uiDataLength, PUINT32 puiWritten);
BOOLEAN readPipeContext(PEXAMPLE_LIST pExampleList, PCHAR pcData,
                        UINT32 uiDataLength, PUINT32 puiWritten);

BOOLEAN isStringTerminated(PCHAR pStr, UINT32 uiStrLen);

#pragma alloc_text(PAGE, handleSampleIoctlBufferedIo)
#pragma alloc_text(PAGE, createPipeContext)
#pragma alloc_text(PAGE, releasePipeContext)
#pragma alloc_text(PAGE, handleSampleIoctlBufferedIo)
#pragma alloc_text(PAGE, writePipeContext)
#pragma alloc_text(PAGE, readPipeContext)

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

/**
 * Communicating with the user mode CloseHandle() function.
 *
 * When this function is called, a pipe context instance reference will be
 * removed.
 *
 * @param pDriverObject Pointer to the current driver object
 * @param pIrp Pointer to the IRP structure
 *
 */
NTSTATUS handleCloseFile(PDEVICE_OBJECT pDeviceObject, PIRP pIrp) {
  NTSTATUS NtStatus = STATUS_SUCCESS;
  PIO_STACK_LOCATION pIoStackIrp = NULL;

  pIoStackIrp = IoGetCurrentIrpStackLocation(pIrp);

  DbgPrint("handleCloseFile() called\n");

  NtStatus = releasePipeContext(
      (PEXAMPLE_DEVICE_CONTEXT)pDeviceObject->DeviceExtension,
      pIoStackIrp->FileObject);
  pIrp->IoStatus.Status = NtStatus;

  IoCompleteRequest(pIrp, IO_NO_INCREMENT);

  return NtStatus;
}

/**
 *
 * Release a pipe context instance and delete it when no reference is left.
 *
 * @param pExampleDeviceContext Pointer to the device context created along with
 * the device, containing all information needed to create the pipe context
 * instance
 * @param pFileObject Pointer to the file object structure in the IRP, refers to
 * the new pipe context object created
 *
 */
NTSTATUS releasePipeContext(PEXAMPLE_DEVICE_CONTEXT pExampleDeviceContext,
                            PFILE_OBJECT pFileObject) {
  NTSTATUS NtStatus = STATUS_UNSUCCESSFUL;
  PEXAMPLE_LIST pExampleList = NULL,
                pExampleListFromIrp = (PEXAMPLE_LIST)pFileObject->FsContext;
  BOOLEAN bNotFound = FALSE;

  DbgPrint("releasePipeContext() called\n");

  NtStatus = KeWaitForSingleObject(&pExampleDeviceContext->kmListMutex,
                                   Executive, KernelMode, FALSE, NULL);

  if (NT_SUCCESS(NtStatus)) {
    pExampleList = pExampleDeviceContext->pExampleList;
    if (pExampleListFromIrp) {
      if (pExampleList == pExampleListFromIrp) {
        bNotFound = FALSE;
        pExampleListFromIrp->uiRefCount--;
        DbgPrint("A reference to pipe %wZ is removed\n",
                 &pExampleList->usPipeName);

        if (pExampleListFromIrp->uiRefCount == 0) {
          DbgPrint(
              "The %wZ pipe context instance will be deleted as no reference is "
              "left\n",
              &pExampleList->usPipeName);
          pExampleDeviceContext->pExampleList = pExampleList->pNext;
          ExFreePool(pExampleListFromIrp);
          DbgPrint("Delete %wZ pipe context instance successful\n",
                   &pExampleList->usPipeName);
        }
      } else {
        do {
          if (pExampleListFromIrp == pExampleList->pNext) {
            bNotFound = FALSE;
            pExampleListFromIrp->uiRefCount--;
            DbgPrint("A reference to pipe %wZ is removed\n",
                     &pExampleList->usPipeName);

            if (pExampleListFromIrp->uiRefCount == 0) {
              DbgPrint(
                  "The %wZ pipe context instance will be deleted as no "
                  "reference is left\n",
                  &pExampleList->usPipeName);
              pExampleDeviceContext->pExampleList = pExampleList->pNext;
              ExFreePool(pExampleListFromIrp);
              DbgPrint("Delete %wZ pipe context instance successful\n",
                       &pExampleList->usPipeName);
            }

            NtStatus = STATUS_SUCCESS;
          } else {
            pExampleList = pExampleList->pNext;
          }
        } while (pExampleList && bNotFound);
      }
    }

    if (bNotFound) {
      DbgPrint(
          "Release pipe %wZ failed because no reference of the pipe was found\n",
          &pExampleList->usPipeName);
      NtStatus = STATUS_UNSUCCESSFUL;
    }

    KeReleaseMutex(&pExampleDeviceContext->kmListMutex, FALSE);
  }
  return NtStatus;
}

/**
 * Communicating with the user mode CreateFile() function.
 *
 * A pipe context instance will be created when calling this function. See
 * createPipeContext for more details
 *
 * @param pDriverObject Pointer to the current driver object
 * @param pIrp Pointer to the IRP structure
 *
 */
NTSTATUS handleCreateFile(PDEVICE_OBJECT pDeviceObject, PIRP pIrp) {
  NTSTATUS NtStatus = STATUS_UNSUCCESSFUL;
  PIO_STACK_LOCATION pIoStackIrp = NULL;

  pIoStackIrp = IoGetCurrentIrpStackLocation(pIrp);

  DbgPrint("handleCreateFile() called\n");

  NtStatus =
      createPipeContext((PEXAMPLE_DEVICE_CONTEXT)pDeviceObject->DeviceExtension,
                        pIoStackIrp->FileObject);
  pIrp->IoStatus.Status = NtStatus;

  DbgPrint("Return status is 0x%08x\n", NtStatus);

  IoCompleteRequest(pIrp, IO_NO_INCREMENT);

  return NtStatus;
}

/**
 *
 * Find a pipe context instance, or create a new one if that does not exist.
 *
 * @param pExampleDeviceContext Pointer to the device context created along with
 * the device, containing all information needed to create the pipe context
 * instance
 * @param pFileObject Pointer to the file object structure in the IRP, refers to
 * the new pipe context instance object created
 *
 */
NTSTATUS createPipeContext(PEXAMPLE_DEVICE_CONTEXT pExampleDeviceContext,
                           PFILE_OBJECT pFileObject) {
  NTSTATUS NtStatus = STATUS_UNSUCCESSFUL;
  PEXAMPLE_LIST pExampleList = NULL;
  BOOLEAN bNeedsToCreate = FALSE;

  DbgPrint("createPipeContext() called\n");

  NtStatus = KeWaitForMutexObject(&pExampleDeviceContext->kmListMutex, Executive,
                                  KernelMode, FALSE, NULL);

  if (NT_SUCCESS(NtStatus)) {
    pExampleList = pExampleDeviceContext->pExampleList;
    bNeedsToCreate = TRUE;

    if (pExampleList) {
      do {
        if (RtlCompareUnicodeString(&pExampleList->usPipeName,
                                    &pFileObject->FileName, TRUE) == 0) {
          // If a pipe context instance with the same name already exists, we do
          // not need to create a new pipe context instance, instead we add 1 to
          // the reference count
          bNeedsToCreate = FALSE;
          pExampleList->uiRefCount++;
          pFileObject->FsContext = (PVOID)pExampleList;

          NtStatus = STATUS_SUCCESS;
        } else {
          pExampleList = pExampleList->pNext;
        }
      } while (pExampleList && bNeedsToCreate);
    }

    if (bNeedsToCreate) {
      DbgPrint(
          "Pipe %wZ does not exist. A new pipe context instance will be "
          "created\n",
          &pExampleList->usPipeName);
      pExampleList = (PEXAMPLE_LIST)ExAllocatePoolWithTag(
          NonPagedPool, sizeof(EXAMPLE_LIST), EXAMPLE_POOL_TAG);

      if (pExampleList) {
        pExampleList->pNext = pExampleDeviceContext->pExampleList;
        pExampleDeviceContext->pExampleList = pExampleList;

        pExampleList->uiRefCount = 1;
        pExampleList->usPipeName.Length = 0;
        pExampleList->usPipeName.MaximumLength =
            sizeof(pExampleList->pwcPipeName);
        pExampleList->usPipeName.Buffer = pExampleList->pwcPipeName;
        pExampleList->uiStartIndex = 0;
        pExampleList->uiStopIndex = 0;

        KeInitializeMutex(&pExampleList->kmInstanceBufferMutex, 0);

        RtlCopyUnicodeString(&pExampleList->usPipeName, &pFileObject->FileName);

        pFileObject->FsContext = (PVOID)pExampleList;

        NtStatus = STATUS_SUCCESS;

        DbgPrint("Create pipe %wZ successful\n", &pExampleList->usPipeName);
      } else {
        DbgPrint("Create pipe %wZ failed due to insufficient resources\n",
                 &pExampleList->usPipeName);
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
      }

      
    } else {
      DbgPrint("Added 1 reference of pipe %wZ\n", &pExampleList->usPipeName);
    }
    DbgPrint("%Instance count: %i\n", pExampleList->uiRefCount);
    KeReleaseMutex(&pExampleDeviceContext->kmListMutex, FALSE);
  }
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
        // LARGE_INTEGER DelayTime;
        // DelayTime.QuadPart = 10000000000;
        // KeDelayExecutionThread(KernelMode, FALSE, &DelayTime);

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
  UINT32 uiDataRead = 0;
  PCHAR pReadDataBuffer;

  DbgPrint("handleReadFile() Called.\r\n");

  pIoStackIrp = IoGetCurrentIrpStackLocation(pIrp);

  if (pIoStackIrp) {
    DbgPrint("Preparing to modify the buffer\n");
    pReadDataBuffer = (PCHAR)pIrp->AssociatedIrp.SystemBuffer;
    if (pReadDataBuffer && pIoStackIrp->Parameters.Read.Length > 0) {
      DbgPrint("Started writing to the buffer\n");

      if (readPipeContext((PEXAMPLE_LIST)pIoStackIrp->FileObject->FsContext,
                          pReadDataBuffer, pIoStackIrp->Parameters.Read.Length,
                          &uiDataRead)) {
        DbgPrint("Data got from user mode buffer: %ws\n", pReadDataBuffer);
        NtStatus = STATUS_SUCCESS;
      }
    }
  }

  pIrp->IoStatus.Status = NtStatus;
  pIrp->IoStatus.Information = uiDataRead;

  // Priority boost to make the thread waiting for this IRP to complete
  IoCompleteRequest(pIrp, IO_NO_INCREMENT);

  return NtStatus;
#else

#endif  //__USE_BUFFERED__
#endif  //__USE_DIRECT__
}

/**
 * Write data to the pipe from the circular buffer.
 *
 * The data is read from the buffer space between the start index and the stop
 * index (in that order). The function does not block.
 *
 * @param[in] pExampleList Pointer to list of pipe context instances
 * @param[in] pcData Pointer to the data buffer needs reading
 * @param[in] uiDataLength Length of the data needs reading
 * @param[out] puiRead Pointer to a UINT variable indicates the amount of
 * data read from the buffer
 *
 * @return TRUE if any data was read
 *
 */
BOOLEAN readPipeContext(PEXAMPLE_LIST pExampleList, PCHAR pcData,
                        UINT32 uiDataLength, PUINT32 puiRead) {
  BOOLEAN bIsDataRead = FALSE;
  NTSTATUS NtStatus = STATUS_SUCCESS;

  *puiRead = 0;

  // This reading process should not be shared with any other working threads
  NtStatus = KeWaitForSingleObject(&pExampleList->kmInstanceBufferMutex,
                                   Executive, KernelMode, FALSE, NULL);

  if (NT_SUCCESS(NtStatus)) {
    DbgPrint("Start Index = %i Stop Index = %i Size Of Buffer = %i\n",
             pExampleList->uiStartIndex, pExampleList->uiStopIndex,
             sizeof(pExampleList->pcCircularBuffer));

    DbgPrint("readPipeContext = %i > %i\n", pExampleList->uiStartIndex,
             pExampleList->uiStopIndex);
    if (pExampleList->uiStartIndex < pExampleList->uiStopIndex) {
      // Case: Start position < stop position
      // [    <Start>****************<Stop>                     ]
      // In this case, we copy the data between the indices linearly
      UINT32 uiCopyLength = MIN(
          pExampleList->uiStopIndex - pExampleList->uiStartIndex, uiDataLength);
      DbgPrint("uiCopyLength = %i (%i, %i)\n", uiCopyLength,
               (pExampleList->uiStopIndex - pExampleList->uiStartIndex),
               uiDataLength);
      if (uiCopyLength) {
        RtlCopyMemory(
            pcData, pExampleList->pcCircularBuffer + pExampleList->uiStartIndex,
            uiCopyLength);
        pExampleList->uiStartIndex += uiCopyLength;
        *puiRead = uiCopyLength;
        bIsDataRead = TRUE;
      }
    } else {
      // Case: Start position > stop position
      // [*****<Stop>              <Start>************************]
      // We will try to copy the memory at the end of the buffer first. Then, we
      // wrap around, and continue to copy memory if start != stop
      UINT32 uiLinearLengthAvailable;
      UINT32 uiCopyLength;
      if (pExampleList->uiStartIndex > pExampleList->uiStopIndex) {
        // If start == stop, no data is available. The function should return
        // FALSE
        uiLinearLengthAvailable =
            sizeof(pExampleList->pcCircularBuffer) - pExampleList->uiStartIndex;

        uiCopyLength = MIN(uiLinearLengthAvailable, uiDataLength);

        DbgPrint(
            "uiCopyLength %i = MIN(uiLinearLengthAvailable %i, uiDataLength "
            "%i)\n",
            uiCopyLength, uiLinearLengthAvailable, uiDataLength);

        if (uiCopyLength) {
          RtlCopyMemory(
              pcData,
              pExampleList->pcCircularBuffer + pExampleList->uiStartIndex,
              uiCopyLength);
          pExampleList->uiStopIndex += uiCopyLength;
          *puiRead = uiCopyLength;
          bIsDataRead = TRUE;

          if (pExampleList->uiStartIndex ==
              sizeof(pExampleList->pcCircularBuffer)) {
            // When the start index exceed the buffer size, we have to wrap
            // around...
            pExampleList->uiStartIndex = 0;
            DbgPrint("pExampleList->uiStartIndex = 0 %i - %i = %i\n",
                     uiDataLength, uiCopyLength, (uiDataLength - uiCopyLength));

            // ... and read any data left
            if (uiDataLength - uiCopyLength > 0) {
              UINT32 uiSecondCopyLength =
                  MIN(pExampleList->uiStopIndex - pExampleList->uiStartIndex,
                      uiDataLength - uiCopyLength);
              DbgPrint("uiSecondCopyLength = 0 %i\n", uiSecondCopyLength);
              if (uiSecondCopyLength) {
                RtlCopyMemory(
                    pcData,
                    pExampleList->pcCircularBuffer + pExampleList->uiStartIndex,
                    uiSecondCopyLength);
                pExampleList->uiStartIndex += uiSecondCopyLength;
                *puiRead = uiCopyLength + uiSecondCopyLength;
                bIsDataRead = TRUE;
              }
            }
          }
        }
      }
    }
    DbgPrint("Start Index = %i Stop Index = %i Size Of Buffer = %i\n",
             pExampleList->uiStartIndex, pExampleList->uiStopIndex,
             sizeof(pExampleList->pcCircularBuffer));
    KeReleaseMutex(&pExampleList->kmInstanceBufferMutex, FALSE);
  }
  return bIsDataRead;
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
  UINT32 uiDataWritten = 0;

  DbgPrint("handleWriteFile() Called. Mode: Buffered I/O \r\n");

  pIoStackIrp = IoGetCurrentIrpStackLocation(pIrp);

  if (pIoStackIrp) {
    pWriteDataBuffer = (PCHAR)pIrp->AssociatedIrp.SystemBuffer;
    if (pWriteDataBuffer) {
      DbgPrint("Data got from user mode buffer: %ws\n", pWriteDataBuffer);
      if (writePipeContext((PEXAMPLE_LIST)pIoStackIrp->FileObject->FsContext,
                           pWriteDataBuffer,
                           pIoStackIrp->Parameters.Write.Length,
                           &uiDataWritten)) {
        NtStatus = STATUS_SUCCESS;
      }
    }
    DbgPrint("\n");
  }

  pIrp->IoStatus.Status = NtStatus;
  pIrp->IoStatus.Information = uiDataWritten;

  // Priority boost to make the thread waiting for this IRP to complete
  IoCompleteRequest(pIrp, IO_NO_INCREMENT);

  return NtStatus;
#else

#endif  //__USE_BUFFERED__
#endif  //__USE_DIRECT__
}

/**
 * Write data from the pipe to the circular buffer.
 *
 * The data is written to the buffer space between the stop index and the start
 * index (in that order). The function does not block and will not overwrite
 * data, so excess data will fail.
 *
 * @param[in] pExampleList Pointer to list of pipe context instances
 * @param[in] pcData Pointer to the data buffer needs writing
 * @param[in] uiDataLength Length of the data needs writing
 * @param[out] puiRead Pointer to a UINT variable indicates the amount of
 * data written to the buffer
 *
 * @return TRUE if any data was written to the buffer
 *
 */
BOOLEAN writePipeContext(PEXAMPLE_LIST pExampleList, PCHAR pcData,
                         UINT32 uiDataLength, PUINT32 puiWritten) {
  BOOLEAN bIsDataWritten = FALSE;
  NTSTATUS NtStatus = STATUS_SUCCESS;

  *puiWritten = 0;

  // This writing process should not be shared with any other working threads
  NtStatus = KeWaitForSingleObject(&pExampleList->kmInstanceBufferMutex,
                                   Executive, KernelMode, FALSE, NULL);
  if (NT_SUCCESS(NtStatus)) {
    DbgPrint("Start Index = %i Stop Index = %i Size Of Buffer = %i\n",
             pExampleList->uiStartIndex, pExampleList->uiStopIndex,
             sizeof(pExampleList->pcCircularBuffer));

    DbgPrint("Example_WriteData = %i > %i\n", pExampleList->uiStartIndex,
             (pExampleList->uiStopIndex + 1));
    if (pExampleList->uiStartIndex > (pExampleList->uiStopIndex + 1)) {
      // Case: Start position > stop position
      // [*****<Stop>              <Start>************************]
      // In this case, data is written linearly to the space between those two
      // indices. We cannot make start = stop, in that case the buffer is
      // seemingly empty. Therefore, the writing space is actually stop to
      // start - 1.
      // The minus operation is written in the form (start - (stop +
      // 1)) to avoid negative values.
      UINT32 uiCopyLength =
          MIN(pExampleList->uiStartIndex - (pExampleList->uiStopIndex + 1),
              uiDataLength);
      DbgPrint("uiCopyLength = %i (%i, %i)\n", uiCopyLength,
               (pExampleList->uiStartIndex - (pExampleList->uiStopIndex + 1)),
               uiDataLength);
      if (uiCopyLength) {
        RtlCopyMemory(
            pExampleList->pcCircularBuffer + pExampleList->uiStopIndex, pcData,
            uiCopyLength);
        pExampleList->uiStopIndex += uiCopyLength;
        *puiWritten = uiCopyLength;
        bIsDataWritten = TRUE;
      }
    } else {
      // Case: Start position < stop position
      // [    <Start>****************<Stop>                     ]
      // In this case, we will write the the area from the stop index to the end
      // of the buffer. Once more, the write space is actually stop to start -
      // 1, due to the same reason as above.
      UINT32 uiLinearLengthAvailable;
      UINT32 uiCopyLength;
      if (pExampleList->uiStartIndex <= pExampleList->uiStopIndex) {
        if (pExampleList->uiStartIndex == 0) {
          // When start = 0, we will not wrap around, and assume that the last
          // position of the buffer is the start position
          uiLinearLengthAvailable = sizeof(pExampleList->pcCircularBuffer) -
                                    (pExampleList->uiStopIndex + 1);

        } else {
          uiLinearLengthAvailable = sizeof(pExampleList->pcCircularBuffer) -
                                    pExampleList->uiStopIndex;
        }
        uiCopyLength = MIN(uiLinearLengthAvailable, uiDataLength);

        DbgPrint(
            "uiCopyLength %i = MIN(uiLinearLengthAvailable %i, uiDataLength "
            "%i)\n",
            uiCopyLength, uiLinearLengthAvailable, uiDataLength);

        if (uiCopyLength) {
          RtlCopyMemory(
              pExampleList->pcCircularBuffer + pExampleList->uiStopIndex,
              pcData, uiCopyLength);
          pExampleList->uiStopIndex += uiCopyLength;
          *puiWritten = uiCopyLength + uiCopyLength;
          bIsDataWritten = TRUE;

          if (pExampleList->uiStopIndex ==
              sizeof(pExampleList->pcCircularBuffer)) {
            // When the stop index exceed the buffer size (this will not happen
            // if start = 0), we have to wrap around...
            pExampleList->uiStopIndex = 0;
            DbgPrint("pExampleList->uiStopIndex = 0 %i - %i = %i\n",
                     uiDataLength, uiCopyLength, (uiDataLength - uiCopyLength));

            // ... and write any data left
            if (uiDataLength - uiCopyLength > 0) {
              UINT32 uiSecondCopyLength = MIN(
                  pExampleList->uiStartIndex - (pExampleList->uiStopIndex + 1),
                  uiDataLength - uiCopyLength);
              DbgPrint("uiSecondCopyLength = 0 %i\n", uiSecondCopyLength);
              if (uiSecondCopyLength) {
                RtlCopyMemory(
                    pExampleList->pcCircularBuffer + pExampleList->uiStopIndex,
                    pcData, uiSecondCopyLength);
                pExampleList->uiStopIndex += uiSecondCopyLength;
                *puiWritten = uiCopyLength + uiSecondCopyLength;
                bIsDataWritten = TRUE;
              }
            }
          }
        }
      }
    }
    DbgPrint("Start Index = %i Stop Index = %i Size Of Buffer = %i\n",
             pExampleList->uiStartIndex, pExampleList->uiStopIndex,
             sizeof(pExampleList->pcCircularBuffer));
    DbgPrint("Buffer content %ws\n", (PWCHAR)pExampleList->pcCircularBuffer);
    KeReleaseMutex(&pExampleList->kmInstanceBufferMutex, FALSE);
  }
  return bIsDataWritten;
}