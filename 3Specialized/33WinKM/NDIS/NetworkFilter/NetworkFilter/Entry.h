#pragma once
#define NDIS_WDM
#define NDIS689

#include <wdm.h>
#include <fwpsk.h>

#define DbgPrint(x, ...) DbgPrint("[NetworkFilter] " x, __VA_ARGS__)

typedef struct DriverData_ {
  PDEVICE_OBJECT pDeviceObject;
  UINT32 uiCalloutId;
} DriverData, *PDriverData;

DriverData driverData;

VOID DriverUnload(PDRIVER_OBJECT pDriverObject);

NTSTATUS FwpsCalloutClassify(const FWPS_INCOMING_VALUES *pInFixedValues,
                             const FWPS_INCOMING_METADATA_VALUES *pInMetaValues,
                             void *pLayerData, const void *pClassifyContext,
                             const FWPS_FILTER *pFilter, UINT64 ui64flowContext,
                             FWPS_CLASSIFY_OUT0 *pClassifyOut);

NTSTATUS FwpsCalloutNotify(const FWPS_CALLOUT_NOTIFY_TYPE notifyType,
                           const GUID *filterKey, const FWPS_FILTER *filter);

NTSTATUS FwpsFlowDeleteNotify(UINT16 layerId, UINT32 calloutId,
                              UINT64 flowContext);

CONST FWPS_CALLOUT kCalloutRegistration = {
    // 6d8dbf75-ffcd-4799-aa62-10b9e083163f
    .calloutKey = {0x6d8dbf75,
                   0xffcd,
                   0x4799,
                   {0xaa, 0x62, 0x10, 0xb9, 0xe0, 0x83, 0x16, 0x3f}},
    .flags = 0,
    .classifyFn = FwpsCalloutClassify,
    .notifyFn = FwpsCalloutNotify,
    .flowDeleteFn = FwpsFlowDeleteNotify,
};