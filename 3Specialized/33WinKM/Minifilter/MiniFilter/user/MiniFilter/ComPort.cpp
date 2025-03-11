#include "ComPort.hpp"

ComPort::ComPort() { hComPort = NULL; }

ComPort::~ComPort() {
  if (hComPort != NULL) {
    // CloseHandle(hComPort);
  }
  hComPort = NULL;
}

HRESULT ComPort::connectToKernelNode(std::wstring sPortName) {
  HRESULT hr = S_OK;
  hr = FilterConnectCommunicationPort(sPortName.c_str(), 0, NULL, 0, NULL,
                                      &hComPort);
  if (hr != S_OK) {
    fwprintf(stderr, L"Connect to kernel mode failed 0x%08x\n", hr);
    fflush(stdout);
    return hr;
  }
  return hr;
}

HRESULT ComPort::getRecord(PMFLT_EVENT_RECORD pEventRecord) {
  HRESULT hr = S_OK;
  COM_MESSAGE msg;
  memset(&msg, 0, sizeof(&msg));
  hr = FilterGetMessage(
      hComPort, &msg.header,
      sizeof(MFLT_EVENT_RECORD) + sizeof(FILTER_MESSAGE_HEADER), NULL);
  if (hr != S_OK) {
    fwprintf(stderr, L"Get message failed 0x%08x\n", hr);
    return hr;
  }
  // fwprintf(stderr, L"Get message OK\n");
  // fflush(stdout);

  // fwprintf(stderr, L"0x%08x\n", pEventRecord);
  // fwprintf(stderr, L"%d\n", sizeof(MFLT_EVENT_RECORD));

  memcpy_s(pEventRecord, sizeof(MFLT_EVENT_RECORD), &msg.eventRecord,
           sizeof(MFLT_EVENT_RECORD));

  // fwprintf(stderr, L"Copy memory OK\n");
  // fflush(stdout);

  // mtx.lock();
  // fo << std::wstring(pwcObtainedMsgContent);
  // mtx.unlock();
  return hr;
}

//HRESULT ComPort::getRecord(std::vector<MFLT_EVENT_RECORD>* pvEventRecord) {
//  HRESULT hr = S_OK;
//  COM_MESSAGE msg;
//  memset(&msg, 0, sizeof(&msg));
//  hr = FilterGetMessage(
//      hComPort, &msg.header,
//      sizeof(MFLT_SEND_MESSAGE) + sizeof(FILTER_MESSAGE_HEADER), NULL);
//  if (hr != S_OK) {
//    fwprintf(stderr, L"Get message failed 0x%08x\n", hr);
//    return hr;
//  }
//  // fwprintf(stderr, L"Get message OK\n");
//  // fflush(stdout);
//
//  // fwprintf(stderr, L"0x%08x\n", pEventRecord);
//  // fwprintf(stderr, L"%d\n", sizeof(MFLT_EVENT_RECORD));
//
//  for (ULONG uiCurrentRecordId = 0;
//       uiCurrentRecordId < msg.sendMessage.uiEventRecordCount;
//       uiCurrentRecordId++) {
//    pvEventRecord->push_back(
//        msg.sendMessage.pEventRecordBuffer[uiCurrentRecordId]);
//  }
//
//  // fwprintf(stderr, L"Copy memory OK\n");
//  // fflush(stdout);
//
//  // mtx.lock();
//  // fo << std::wstring(pwcObtainedMsgContent);
//  // mtx.unlock();
//  return hr;
//}

HRESULT ComPort::disconnectFromKernelMode() {
  CloseHandle(hComPort);
  return S_OK;
}