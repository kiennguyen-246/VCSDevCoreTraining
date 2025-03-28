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

  RtlInitUnicodeString(&symbolicLinkName, L"\\DosDevices\\NetworkFilter");
  IoDeleteSymbolicLink(&symbolicLinkName);
  IoDeleteDevice(driverData.pDeviceObject);
}

NTSTATUS FwpsCalloutClassify(const FWPS_INCOMING_VALUES *pInFixedValues,
                             const FWPS_INCOMING_METADATA_VALUES *pInMetaValues,
                             void *pLayerData, const void *pClassifyContext,
                             const FWPS_FILTER *pFilter, UINT64 ui64flowContext,
                             FWPS_CLASSIFY_OUT0 *pClassifyOut) {
  UNREFERENCED_PARAMETER(pInMetaValues);
  UNREFERENCED_PARAMETER(pLayerData);
  UNREFERENCED_PARAMETER(pClassifyContext);
  UNREFERENCED_PARAMETER(pFilter);
  UNREFERENCED_PARAMETER(ui64flowContext);
  UNREFERENCED_PARAMETER(pClassifyOut);

  DbgPrint("FwpsCalloutClassify called\n");
  DbgPrint("Current event is at layer %d\n", pInFixedValues->layerId);

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