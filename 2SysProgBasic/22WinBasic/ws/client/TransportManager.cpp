#include "TransportManager.hpp"

TransportManager::TransportManager() { qwsData.empty(); }

TransportManager::~TransportManager() {}

HRESULT TransportManager::tmWriteData(LPBYTE pbData, PULONG puiDataLength) {
  HRESULT hr = S_OK;
  if (pbData == NULL) {
    std::wcout << "No data is written\n";
    return hr;
  }

  LPBYTE buffer = new (std::nothrow) BYTE[TM_MAX_BUFFER_SIZE];
  if (buffer == NULL) {
    hr = E_OUTOFMEMORY;
    std::wcout << "Cannot allocate memory for the buffer";
    return hr;
  }
  memset(buffer, 0, sizeof(buffer));

  memcpy(buffer, pbData, *puiDataLength);
  std::wstring wsDataEntry = (LPWSTR)buffer;

  mtx.lock();
  qwsData.push(wsDataEntry);
  mtx.unlock();

  return hr;
}

HRESULT TransportManager::tmReadData(LPBYTE pbData, PULONG puiDataLength) {
  HRESULT hr = S_OK;
  *puiDataLength = 0;
  if (pbData == NULL) {
    hr = E_FAIL;
    return hr;
  }

  BYTE buffer[TM_MAX_BUFFER_SIZE];

  mtx.lock();

  while (!qwsData.empty()) {
    auto wsCurrentEntry = qwsData.front();
    auto uiSizeOfCurrentEntry = wsCurrentEntry.length() * sizeof(WCHAR);
    if (*puiDataLength + uiSizeOfCurrentEntry <= TM_MAX_BUFFER_SIZE) {
      qwsData.pop();
      memset(buffer, 0, sizeof(buffer));
      wsCurrentEntry.push_back(L'\n');
      memcpy(pbData + *puiDataLength, wsCurrentEntry.c_str(),
             uiSizeOfCurrentEntry);
      *puiDataLength += uiSizeOfCurrentEntry;
    } else {
      break;
    }
    if (*puiDataLength > 0) {
      for (ULONG i = 1; i <= sizeof(WCHAR); i++) {
        pbData[*puiDataLength - i] = 0;
      }
    } else {
      hr = E_FAIL;
    }
    
  }

  mtx.unlock();
  return hr;

}
