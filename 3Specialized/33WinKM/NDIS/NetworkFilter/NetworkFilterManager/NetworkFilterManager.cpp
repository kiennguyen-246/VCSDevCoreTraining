// NetworkFilterManager.cpp : This file contains the 'main' function. Program
// execution begins and ends there.
//

#include <Windows.h>
#include <fwpmu.h>

#include <iostream>

#define GUID_NULL \
  { 0, 0, 0, NULL }
#define ERROR_MESSAGE_BOX(message) \
  MessageBox(NULL, message, L"Error", MB_OK | MB_ICONSTOP)
#define INFO_MESSAGE_BOX(message) \
  MessageBox(NULL, message, L"Information", MB_OK | MB_ICONINFORMATION)

// {DEEA69EF-B800-42EA-A13E-826F8D18F761}
CONST GUID kProviderGUID = {0xdeea69ef,
                            0xb800,
                            0x42ea,
                            {0xa1, 0x3e, 0x82, 0x6f, 0x8d, 0x18, 0xf7, 0x61}};

// {51EE1811-AC53-4BEE-9827-D26B0F40ED02}
CONST GUID kSublayerGUID = {
    0x51ee1811, 0xac53, 0x4bee, {0x98, 0x27, 0xd2, 0x6b, 0xf, 0x40, 0xed, 0x2}};

// 6d8dbf75-ffcd-4799-aa62-10b9e083163f
CONST GUID kCalloutGUID = {0x6d8dbf75,
                           0xffcd,
                           0x4799,
                           {0xaa, 0x62, 0x10, 0xb9, 0xe0, 0x83, 0x16, 0x3f}};

CONST WCHAR kSessionName[] = L"Network Filter Manager Session";
CONST WCHAR kCalloutName[] = L"Network Filter Callout";
CONST WCHAR kInspectionFilterName[] = L"Connecting layer inspection";

DWORD installProvider(CONST GUID* pProviderKey, PCWCHAR pwcProviderName,
                      CONST GUID* pSublayerKey, PCWCHAR pwcSublayerName) {
  DWORD dwRes;

  HANDLE hEngine = NULL;
  FWPM_SESSION fwpSession;
  ZeroMemory(&fwpSession, sizeof(fwpSession));
  fwpSession.displayData.name = (PWCHAR)kSessionName;
  fwpSession.txnWaitTimeoutInMSec = INFINITE;
  dwRes =
      FwpmEngineOpen(NULL, RPC_C_AUTHN_DEFAULT, NULL, &fwpSession, &hEngine);
  if (dwRes != ERROR_SUCCESS) {
    WCHAR acErrorMessageBuffer[MAX_PATH];
    swprintf_s(acErrorMessageBuffer, MAX_PATH,
               L"FwpmEngineOpen failed 0x%08x\n", dwRes);
    ERROR_MESSAGE_BOX(acErrorMessageBuffer);
    return dwRes;
  }

  dwRes = FwpmTransactionBegin(hEngine, 0);
  if (dwRes != ERROR_SUCCESS) {
    WCHAR acErrorMessageBuffer[MAX_PATH];
    swprintf_s(acErrorMessageBuffer, MAX_PATH,
               L"FwpmTransactionBegin failed 0x%08x\n", dwRes);
    ERROR_MESSAGE_BOX(acErrorMessageBuffer);
    return dwRes;
  }

  FWPM_PROVIDER fwpProvider;
  ZeroMemory(&fwpProvider, sizeof(fwpProvider));
  fwpProvider.providerKey = *pProviderKey;
  fwpProvider.displayData.name = (PWCHAR)pwcProviderName;
  fwpProvider.flags = FWPM_PROVIDER_FLAG_PERSISTENT;
  dwRes = FwpmProviderAdd(hEngine, &fwpProvider, NULL);
  if (dwRes != ERROR_SUCCESS) {
    WCHAR acErrorMessageBuffer[MAX_PATH];
    swprintf_s(acErrorMessageBuffer, MAX_PATH,
               L"FwpmProviderAdd failed 0x%08x\n", dwRes);
    ERROR_MESSAGE_BOX(acErrorMessageBuffer);
    return dwRes;
  }

  FWPM_SUBLAYER fwpSublayer;
  ZeroMemory(&fwpSublayer, sizeof(fwpSublayer));
  fwpSublayer.subLayerKey = *pSublayerKey;
  fwpSublayer.displayData.name = (PWCHAR)pwcSublayerName;
  fwpSublayer.flags = FWPM_SUBLAYER_FLAG_PERSISTENT;
  fwpSublayer.providerKey = (GUID*)pProviderKey;
  fwpSublayer.weight = 0x8000;
  dwRes = FwpmSubLayerAdd(hEngine, &fwpSublayer, NULL);
  if (dwRes != ERROR_SUCCESS && dwRes != FWP_E_ALREADY_EXISTS) {
    WCHAR acErrorMessageBuffer[MAX_PATH];
    swprintf_s(acErrorMessageBuffer, MAX_PATH,
               L"FwpmSubLayerAdd failed 0x%08x\n", dwRes);
    ERROR_MESSAGE_BOX(acErrorMessageBuffer);
    return dwRes;
  }

  dwRes = FwpmTransactionCommit(hEngine);
  if (dwRes != ERROR_SUCCESS) {
    WCHAR acErrorMessageBuffer[MAX_PATH];
    swprintf_s(acErrorMessageBuffer, MAX_PATH,
               L"FwpmTransactionCommit failed 0x%08x\n", dwRes);
    ERROR_MESSAGE_BOX(acErrorMessageBuffer);
    return dwRes;
  }

  FwpmEngineClose(hEngine);

  INFO_MESSAGE_BOX(L"Successfully installed WFP Provider");

  return ERROR_SUCCESS;
}

DWORD uninstallProvider(CONST GUID* pProviderKey, CONST GUID* pSublayerKey) {
  DWORD dwRes;

  HANDLE hEngine = NULL;
  FWPM_SESSION fwpSession;
  ZeroMemory(&fwpSession, sizeof(fwpSession));
  fwpSession.displayData.name = (PWCHAR)kSessionName;
  fwpSession.txnWaitTimeoutInMSec = INFINITE;
  dwRes =
      FwpmEngineOpen(NULL, RPC_C_AUTHN_DEFAULT, NULL, &fwpSession, &hEngine);
  if (dwRes != ERROR_SUCCESS) {
    WCHAR acErrorMessageBuffer[MAX_PATH];
    swprintf_s(acErrorMessageBuffer, MAX_PATH,
               L"FwpmEngineOpen failed 0x%08x\n", dwRes);
    ERROR_MESSAGE_BOX(acErrorMessageBuffer);
    return dwRes;
  }

  dwRes = FwpmTransactionBegin(hEngine, 0);
  if (dwRes != ERROR_SUCCESS) {
    WCHAR acErrorMessageBuffer[MAX_PATH];
    swprintf_s(acErrorMessageBuffer, MAX_PATH,
               L"FwpmTransactionBegin failed 0x%08x\n", dwRes);
    ERROR_MESSAGE_BOX(acErrorMessageBuffer);
    return dwRes;
  }

  dwRes = FwpmSubLayerDeleteByKey(hEngine, pSublayerKey);
  if (dwRes != ERROR_SUCCESS && dwRes != FWP_E_SUBLAYER_NOT_FOUND) {
    WCHAR acErrorMessageBuffer[MAX_PATH];
    swprintf_s(acErrorMessageBuffer, MAX_PATH,
               L"FwpmSubLayerDeleteByKey failed 0x%08x\n", dwRes);
    ERROR_MESSAGE_BOX(acErrorMessageBuffer);
    return dwRes;
  }

  dwRes = FwpmProviderDeleteByKey(hEngine, pProviderKey);
  if (dwRes != ERROR_SUCCESS && dwRes != FWP_E_PROVIDER_NOT_FOUND) {
    WCHAR acErrorMessageBuffer[MAX_PATH];
    swprintf_s(acErrorMessageBuffer, MAX_PATH,
               L"FwpmProviderDeleteByKey failed 0x%08x\n", dwRes);
    ERROR_MESSAGE_BOX(acErrorMessageBuffer);
    return dwRes;
  }

  dwRes = FwpmTransactionCommit(hEngine);
  if (dwRes != ERROR_SUCCESS) {
    WCHAR acErrorMessageBuffer[MAX_PATH];
    swprintf_s(acErrorMessageBuffer, MAX_PATH,
               L"FwpmTransactionCommit failed 0x%08x\n", dwRes);
    ERROR_MESSAGE_BOX(acErrorMessageBuffer);
    return dwRes;
  }

  FwpmEngineClose(hEngine);

  INFO_MESSAGE_BOX(L"Successfully uninstalled WFP Provider");

  return ERROR_SUCCESS;
}

DWORD installCallout(CONST GUID* pProviderGUID, CONST GUID* pSublayerGUID,
                 CONST GUID* pCalloutGUID) {
  DWORD dwRes;

  HANDLE hEngine = NULL;
  FWPM_SESSION fwpSession;
  ZeroMemory(&fwpSession, sizeof(fwpSession));
  fwpSession.displayData.name = (PWCHAR)kSessionName;
  fwpSession.txnWaitTimeoutInMSec = INFINITE;
  dwRes =
      FwpmEngineOpen(NULL, RPC_C_AUTHN_DEFAULT, NULL, &fwpSession, &hEngine);
  if (dwRes != ERROR_SUCCESS) {
    WCHAR acErrorMessageBuffer[MAX_PATH];
    swprintf_s(acErrorMessageBuffer, MAX_PATH,
               L"FwpmEngineOpen failed 0x%08x\n", dwRes);
    ERROR_MESSAGE_BOX(acErrorMessageBuffer);
    return dwRes;
  }

  dwRes = FwpmTransactionBegin(hEngine, 0);
  if (dwRes != ERROR_SUCCESS) {
    WCHAR acErrorMessageBuffer[MAX_PATH];
    swprintf_s(acErrorMessageBuffer, MAX_PATH,
               L"FwpmTransactionBegin failed 0x%08x\n", dwRes);
    ERROR_MESSAGE_BOX(acErrorMessageBuffer);
    return dwRes;
  }

  FWPM_CALLOUT fwpmCallout;
  ZeroMemory(&fwpmCallout, sizeof(fwpmCallout));
  fwpmCallout.calloutKey = kCalloutGUID;
  fwpmCallout.displayData.name = (PWCHAR)kCalloutName;
  fwpmCallout.flags = FWPM_CALLOUT_FLAG_PERSISTENT;
  fwpmCallout.providerKey = (GUID*)pProviderGUID;
  fwpmCallout.applicableLayer = *pSublayerGUID;
  dwRes = FwpmCalloutAdd(hEngine, &fwpmCallout, NULL, NULL);
  if (dwRes != ERROR_SUCCESS) {
    WCHAR acErrorMessageBuffer[MAX_PATH];
    swprintf_s(acErrorMessageBuffer, MAX_PATH,
               L"FwpmCalloutAdd failed 0x%08x\n", dwRes);
    ERROR_MESSAGE_BOX(acErrorMessageBuffer);
    return dwRes;
  }

  FWPM_FILTER fwpmFilter;
  ZeroMemory(&fwpmFilter, sizeof(fwpmFilter));
  fwpmFilter.providerKey = (GUID*)pProviderGUID;
  fwpmFilter.layerKey = *pSublayerGUID;
  fwpmFilter.action.type = FWP_ACTION_CALLOUT_UNKNOWN;
  fwpmFilter.action.calloutKey = *pCalloutGUID;
  fwpmFilter.weight.type = FWP_EMPTY;
  fwpmFilter.numFilterConditions = 0;
  fwpmFilter.displayData.name = (PWCHAR)kInspectionFilterName;
  dwRes = FwpmFilterAdd(hEngine, &fwpmFilter, NULL, NULL);
  if (dwRes != ERROR_SUCCESS) {
    WCHAR acErrorMessageBuffer[MAX_PATH];
    swprintf_s(acErrorMessageBuffer, MAX_PATH,
               L"FwpmFilterAdd failed 0x%08x\n", dwRes);
    ERROR_MESSAGE_BOX(acErrorMessageBuffer);
    return dwRes;
  }

  dwRes = FwpmTransactionCommit(hEngine);
  if (dwRes != ERROR_SUCCESS) {
    WCHAR acErrorMessageBuffer[MAX_PATH];
    swprintf_s(acErrorMessageBuffer, MAX_PATH,
               L"FwpmTransactionCommit failed 0x%08x\n", dwRes);
    ERROR_MESSAGE_BOX(acErrorMessageBuffer);
    return dwRes;
  }

  FwpmEngineClose(hEngine);

  INFO_MESSAGE_BOX(L"Successfully installed callout from the driver");

  return ERROR_SUCCESS;
}

INT main() {
  DWORD dwRes = ERROR_SUCCESS;

  // GUID providerKey = kProviderGUID;
  // GUID sublayerKey = FWPM_LAYER_ALE_AUTH_CONNECT_V4;
  // dwRes = installProvider(&providerKey, L"Network Filter Manager Provider",
  //                         &sublayerKey, L"Sublayer 1");
  // if (dwRes != ERROR_SUCCESS) {
  //   return dwRes;
  // }

   dwRes = installCallout(NULL, &FWPM_LAYER_ALE_AUTH_CONNECT_V4, &kCalloutGUID);

  // dwRes = uninstallProvider(&providerKey, &sublayerKey);
  // if (dwRes != ERROR_SUCCESS) {
  //   return dwRes;
  // }

  return dwRes;
}