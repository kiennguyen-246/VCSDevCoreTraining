// NetworkFilterManager.cpp : This file contains the 'main' function. Program
// execution begins and ends there.
//

#include <Windows.h>
#include <fwpmu.h>

#include <iostream>

#include "Public/event.h"
#include "Public/ioctl.h"

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

// {B3DDEEF8-0DC4-454E-8645-6BB93285A9F1}
CONST GUID kFilterGUID = {0xb3ddeef8,
                          0xdc4,
                          0x454e,
                          {0x86, 0x45, 0x6b, 0xb9, 0x32, 0x85, 0xa9, 0xf1}};

CONST WCHAR kSessionName[] = L"Network Filter Manager Session";
CONST WCHAR kCalloutName[] = L"Network Filter Callout";
CONST WCHAR kInspectionFilterName[] = L"Connecting layer inspection";
CONST WCHAR kDevicePath[] = L"\\\\.\\NetworkFilter";

BOOL bIsStopped = 0;

DWORD installCallout(CONST GUID* pProviderGUID, CONST GUID* pSublayerGUID,
                     CONST GUID* pCalloutGUID, CONST GUID* pFilterGUID) {
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
  fwpmFilter.filterKey = *pFilterGUID;
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
    swprintf_s(acErrorMessageBuffer, MAX_PATH, L"FwpmFilterAdd failed 0x%08x\n",
               dwRes);
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

DWORD uninstallCallout(CONST GUID* pCalloutGUID, CONST GUID* pFilterGUID) {
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

  dwRes = FwpmFilterDeleteByKey(hEngine, pFilterGUID);
  if (dwRes != ERROR_SUCCESS) {
    WCHAR acErrorMessageBuffer[MAX_PATH];
    swprintf_s(acErrorMessageBuffer, MAX_PATH,
               L"FwpmFilterDeleteByKey failed 0x%08x\n", dwRes);
    ERROR_MESSAGE_BOX(acErrorMessageBuffer);
    return dwRes;
  }

  std::wcout << L"Filter deleted\n";

  dwRes = FwpmCalloutDeleteByKey(hEngine, pCalloutGUID);
  if (dwRes != ERROR_SUCCESS) {
    WCHAR acErrorMessageBuffer[MAX_PATH];
    swprintf_s(acErrorMessageBuffer, MAX_PATH,
               L"FwpmCalloutDeleteByKey failed 0x%08x\n", dwRes);
    ERROR_MESSAGE_BOX(acErrorMessageBuffer);
    return dwRes;
  }

  std::wcout << L"Buffer deleted\n";

  dwRes = FwpmTransactionCommit(hEngine);
  if (dwRes != ERROR_SUCCESS) {
    WCHAR acErrorMessageBuffer[MAX_PATH];
    swprintf_s(acErrorMessageBuffer, MAX_PATH,
               L"FwpmTransactionCommit failed 0x%08x\n", dwRes);
    ERROR_MESSAGE_BOX(acErrorMessageBuffer);
    return dwRes;
  }

  std::wcout << L"Committedd\n";

  FwpmEngineClose(hEngine);

  INFO_MESSAGE_BOX(L"Successfully uninstalled callout from the driver");

  return ERROR_SUCCESS;
}

DWORD getEvent(CONST PEvent pRetEvent) {
  DWORD dwErr = 0;

  HANDLE hDevice = CreateFile(kDevicePath, GENERIC_READ | GENERIC_WRITE, 0,
                              NULL, OPEN_EXISTING, 0, NULL);
  CHAR acInputBuffer[] = ".";
  DWORD dwBytesReturned = 0;
  if (!DeviceIoControl(hDevice, IOCTL_GET_EVENT, acInputBuffer,
                       sizeof(acInputBuffer), pRetEvent, sizeof(Event),
                       &dwBytesReturned, NULL)) {
    WCHAR acErrorMessageBuffer[MAX_PATH];
    swprintf_s(acErrorMessageBuffer, MAX_PATH,
               L"DeviceIoControl failed 0x%08x\n", dwErr);
    std::wcout << acErrorMessageBuffer;
    CloseHandle(hDevice);
    return dwErr;
  }

  CloseHandle(hDevice);
  return 0;
}

std::wstring getDottedDecimalAddress(DWORD dwIpAddress) {
  WCHAR awcAddress[16] = L"";
  swprintf_s(awcAddress, L"%d.%d.%d.%d", (dwIpAddress >> 0) & 0xff,
             (dwIpAddress >> 8) & 0xff, (dwIpAddress >> 16) & 0xff,
             (dwIpAddress >> 24) & 0xff);
  std::wstring wsRet = awcAddress;
  return wsRet;
}

VOID logEvent() {
  Event evt;
  if (getEvent(&evt) != 0) {
    return;
  }
  if (evt.kernelHeader.uiIsValid != (UINT64)(-1)) {
    wprintf(L"{\n");
    wprintf(L"\tid: %lu,\n", evt.uiId);
    wprintf(L"\tfilteringLayerId: %lu,\n", evt.uiFilteringLayerId);
    wprintf(L"\tlocalAddress: \"%ws\",\n",
            getDottedDecimalAddress(evt.uiLocalAddress).c_str());
    wprintf(L"\tlocalPort: %lu,\n", evt.uiLocalPort);
    wprintf(L"\tremoteAddress: \"%ws\",\n",
            getDottedDecimalAddress(evt.uiRemoteAddress).c_str());
    wprintf(L"\tremotePort: %lu,\n", evt.uiRemotePort);
    wprintf(L"\tprotocolNumber: %lu,\n", evt.uiProtocolNumber);
    wprintf(L"\tprocessId: %lu,\n", (DWORD)evt.uiProcessId);
    wprintf(L"\tprocessPath: %ws,\n", evt.awcProcessPath);
    wprintf(L"\tsourceInterfaceIndex: %lu,\n", evt.uiSourceInterfaceIndex);
    wprintf(L"\tdestInterfaceIndex: %lu,\n", evt.uiDestInterfaceIndex);
    wprintf(L"},\n");
  }
}

VOID logEventRoutine() {
  while (!bIsStopped) {
    logEvent();
  }

  std::wcout << L"Stopped logging routine\n";
}

INT main() {
  DWORD dwRes = ERROR_SUCCESS;

  /*dwRes = installCallout(NULL, &FWPM_LAYER_ALE_AUTH_CONNECT_V4, &kCalloutGUID,
                         &kFilterGUID);
  if (dwRes) {
    return dwRes;
  }

  std::wcout << L"Filtering events\n";

  HANDLE hThread = CreateThread(NULL, 0, (PTHREAD_START_ROUTINE)logEventRoutine,
                                NULL, 0, NULL);

  while (TRUE) {
    std::wstring wsInp;
    std::wcin >> wsInp;
    if (wsInp == L"stop") {
      bIsStopped = TRUE;
    }
  }

  dwRes = WaitForSingleObject(hThread, 60000);
  if (dwRes) {
    std::wcout << L"Failed to wait for threads\n";
  }

  std::wcout << L"Wait ok\n";*/

  dwRes = uninstallCallout(&kCalloutGUID, &kFilterGUID);
  if (dwRes) {
    return dwRes;
  }

  return dwRes;
}