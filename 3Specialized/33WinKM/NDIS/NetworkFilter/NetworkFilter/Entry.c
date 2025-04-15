#include "Entry.h"

NTSTATUS
DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING puiRegistryPath) {
  UNREFERENCED_PARAMETER(puiRegistryPath);
  DbgPrint("DriverEntry called\n");

  NTSTATUS status = STATUS_SUCCESS;
  UNICODE_STRING deviceName;
  UNICODE_STRING symbolicLinkName;
  RtlInitUnicodeString(&deviceName, L"\\Device\\NetworkFilter");
  RtlInitUnicodeString(&symbolicLinkName, L"\\DosDevices\\NetworkFilter");
  status =
      IoCreateDevice(pDriverObject, 0, &deviceName, FILE_DEVICE_UNKNOWN,
                     FILE_DEVICE_SECURE_OPEN, FALSE, &driverData.pDeviceObject);
  if (!NT_SUCCESS(status)) {
    DbgPrint("IoCreateDevice failed %d", status);
    return status;
  }
  status = IoCreateSymbolicLink(&symbolicLinkName, &deviceName);
  if (!NT_SUCCESS(status)) {
    DbgPrint("IoCreateSymbolicLink failed %d", status);
    IoDeleteDevice(driverData.pDeviceObject);
    return status;
  }

  for (UINT32 i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; ++i) {
    pDriverObject->MajorFunction[i] = IrpMjUnsupported;
  }
  pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IrpMjDeviceControl;

  pDriverObject->DriverUnload = DriverUnload;

  status = FwpsCalloutRegister(pDriverObject, &kCalloutRegistration,
                               &driverData.uiCalloutId);

  if (!NT_SUCCESS(status)) {
    DbgPrint("FwpsCalloutRegister failed with status 0x%x\n", status);
    IoDeleteSymbolicLink(&symbolicLinkName);
    IoDeleteDevice(driverData.pDeviceObject);
    return status;
  }

  return status;
}

VOID DriverUnload(PDRIVER_OBJECT pDriverObject) {
  UNREFERENCED_PARAMETER(pDriverObject);
  DbgPrint("DriverUnload called\n");

  UNICODE_STRING symbolicLinkName;

  NTSTATUS status;
  status = FwpsCalloutUnregisterById(driverData.uiCalloutId);
  if (!NT_SUCCESS(status)) {
    DbgPrint("FwpsCalloutUnregisterById failed with status 0x%x\n", status);
  }

  while (driverData.pFirstEvent != NULL) {
    PEvent pEvt = driverData.pFirstEvent->kernelHeader.pNextEvent;
    ExFreePool(driverData.pFirstEvent);
    driverData.pFirstEvent = pEvt;
  }

  RtlInitUnicodeString(&symbolicLinkName, L"\\DosDevices\\NetworkFilter");
  IoDeleteSymbolicLink(&symbolicLinkName);
  IoDeleteDevice(driverData.pDeviceObject);
}

NTSTATUS IrpMjUnsupported(PDEVICE_OBJECT pDeviceObject, PIRP pIrp) {
  UNREFERENCED_PARAMETER(pDeviceObject);
  UNREFERENCED_PARAMETER(pIrp);

  //DbgPrint("Major function not supported\n");

  return STATUS_NOT_SUPPORTED;
}

NTSTATUS IrpMjDeviceControl(PDEVICE_OBJECT pDeviceObject, PIRP pIrp) {
  UNREFERENCED_PARAMETER(pDeviceObject);
  UNREFERENCED_PARAMETER(pIrp);

  NTSTATUS status = STATUS_SUCCESS;

  PIO_STACK_LOCATION pIoStackIrp = NULL;
  UINT32 uiDataWritten = 0;

  pIoStackIrp = IoGetCurrentIrpStackLocation(pIrp);

  if (pIoStackIrp) {
    switch (pIoStackIrp->Parameters.DeviceIoControl.IoControlCode) {
    case IOCTL_GET_EVENT:
      status = ioctlGetEvent(pIrp, pIoStackIrp, &uiDataWritten);
      break;
    default:
      status = STATUS_NOT_SUPPORTED;
      DbgPrint("IOCTL code is invalid\n");
      break;
    }
  }

  pIrp->IoStatus.Status = status;
  pIrp->IoStatus.Information = uiDataWritten;

  IoCompleteRequest(pIrp, IO_NO_INCREMENT);

  return STATUS_SUCCESS;
}

NTSTATUS ioctlGetEvent(PIRP pIrp, PIO_STACK_LOCATION pIoStackIrp,
                       UINT32 *puiDataWritten) {
  NTSTATUS status = STATUS_UNSUCCESSFUL;

  PBYTE pInputBuffer = pIrp->AssociatedIrp.SystemBuffer;
  PBYTE pOutputBuffer = pIrp->AssociatedIrp.SystemBuffer;
  UINT32 uiDataSize = 0;
  Event evt;
  RtlZeroMemory(&evt, sizeof(Event));

  if (pInputBuffer && pOutputBuffer) {
    if (driverData.pFirstEvent != NULL) {
      evt = *driverData.pFirstEvent;
      evt.kernelHeader.uiIsValid = 1;
      uiDataSize = sizeof(Event);
    } else {
      evt.kernelHeader.uiIsValid = (UINT64)-1;
      uiDataSize = sizeof(Event);
    }

    if (pIoStackIrp->Parameters.DeviceIoControl.OutputBufferLength >=
        uiDataSize) {
      RtlCopyMemory(pOutputBuffer, &evt, uiDataSize);
      *puiDataWritten = uiDataSize;
      status = STATUS_SUCCESS;
    } else {
      *puiDataWritten = uiDataSize;
      status = STATUS_BUFFER_TOO_SMALL;
      DbgPrint("Transfering failed due to too small buffer.\n");
    }

    if (driverData.pFirstEvent != NULL) {
      evt = *driverData.pFirstEvent;
      ExFreePool(driverData.pFirstEvent);
      driverData.pFirstEvent = evt.kernelHeader.pNextEvent;
    }
  } else {
    DbgPrint("Transfering failed due to buffer not found.\n");
  }

  return status;
}

NTSTATUS FwpsCalloutClassify(const FWPS_INCOMING_VALUES *pInFixedValues,
                             const FWPS_INCOMING_METADATA_VALUES *pInMetaValues,
                             void *pLayerData, const void *pClassifyContext,
                             const FWPS_FILTER *pFilter, UINT64 ui64flowContext,
                             FWPS_CLASSIFY_OUT0 *pClassifyOut) {
  UNREFERENCED_PARAMETER(pLayerData);
  UNREFERENCED_PARAMETER(pClassifyContext);
  UNREFERENCED_PARAMETER(pFilter);
  UNREFERENCED_PARAMETER(ui64flowContext);
  UNREFERENCED_PARAMETER(pClassifyOut);

  DbgPrint("FwpsCalloutClassify called\n");

  PEvent pNewEvent =
      ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(Event), 'pefN');
  if (pNewEvent == NULL) {
    DbgPrint("ExAllocatePool2 failed\n");
    return STATUS_BUFFER_TOO_SMALL;
  }

  RtlZeroMemory(pNewEvent, sizeof(Event));
  pNewEvent->kernelHeader.pNextEvent = driverData.pFirstEvent;
  driverData.pFirstEvent = pNewEvent;

  pNewEvent->uiId = ++driverData.uiEventId;
  pNewEvent->uiFilteringLayerId = pInFixedValues->layerId;
  pNewEvent->uiLocalAddress = RtlUlongByteSwap(
      pInFixedValues
          ->incomingValue[FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_LOCAL_ADDRESS]
          .value.uint32);
  pNewEvent->uiLocalPort =
      pInFixedValues
          ->incomingValue[FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_LOCAL_PORT]
          .value.uint32;
  pNewEvent->uiRemoteAddress = RtlUlongByteSwap(
      pInFixedValues
          ->incomingValue[FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_REMOTE_ADDRESS]
          .value.uint32);
  pNewEvent->uiRemotePort =
      pInFixedValues
          ->incomingValue[FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_REMOTE_PORT]
          .value.uint32;
  pNewEvent->uiProtocolNumber =
      pInFixedValues->incomingValue[FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_PROTOCOL]
          .value.uint32;

  if (pInMetaValues->currentMetadataValues & FWPS_METADATA_FIELD_PROCESS_ID) {
    pNewEvent->uiProcessId = pInMetaValues->processId;
  }
  if (pInMetaValues->currentMetadataValues & FWPS_METADATA_FIELD_PROCESS_PATH) {
    pNewEvent->uiProcessPathLength = pInMetaValues->processPath->size;
    RtlCopyMemory(pNewEvent->awcProcessPath, pInMetaValues->processPath->data,
                  pInMetaValues->processPath->size);
  }
  if (pInMetaValues->currentMetadataValues &
      FWPS_METADATA_FIELD_SOURCE_INTERFACE_INDEX) {
    pNewEvent->uiSourceInterfaceIndex = pInMetaValues->sourceInterfaceIndex;
  }
  if (pInMetaValues->currentMetadataValues &
      FWPS_METADATA_FIELD_DESTINATION_INTERFACE_INDEX) {
    pNewEvent->uiDestInterfaceIndex = pInMetaValues->destinationInterfaceIndex;
  }
  // DbgPrint("Current event is at layer %d\n", pInFixedValues->layerId);

  return STATUS_SUCCESS;
}

NTSTATUS FwpsCalloutNotify(const FWPS_CALLOUT_NOTIFY_TYPE notifyType,
                           const GUID *filterKey, const FWPS_FILTER *filter) {
  UNREFERENCED_PARAMETER(notifyType);
  UNREFERENCED_PARAMETER(filterKey);
  UNREFERENCED_PARAMETER(filter);

  DbgPrint("FwpsCalloutNotify called\n");

  return STATUS_SUCCESS;
}

NTSTATUS FwpsFlowDeleteNotify(UINT16 layerId, UINT32 calloutId,
                              UINT64 flowContext) {
  UNREFERENCED_PARAMETER(layerId);
  UNREFERENCED_PARAMETER(calloutId);
  UNREFERENCED_PARAMETER(flowContext);

  DbgPrint("FwpsFlowDeleteNotify called\n");

  return STATUS_SUCCESS;
}