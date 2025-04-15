#include "COM.hpp"

COM::COM() {}

COM::~COM() {}

DWORD COM::setPortName(const std::wstring& __portName) {
  portName = __portName;
  return ERROR_SUCCESS;
}

DWORD COM::connect() {
  HRESULT hr;
  hr = FilterConnectCommunicationPort(portName.c_str(), 0, NULL, 0, NULL,
                                      &hComPort);
  if (hr != S_OK) {
    WCHAR errorMessageBuffer[MAX_PATH];
    swprintf_s(errorMessageBuffer,
               L"FilterConnectCommunicationPort failed 0x%08x", hr);
    Logger::log(LogLevelError, errorMessageBuffer);
    CloseHandle(hComPort);
    return ERROR_GEN_FAILURE;
  }

  BYTE replyBuffer[sizeof(FILTER_MESSAGE_HEADER) + MAX_PATH];
  ZeroMemory(replyBuffer, sizeof(replyBuffer));
  hr = FilterGetMessage(hComPort, (PFILTER_MESSAGE_HEADER)replyBuffer,
                        sizeof(FILTER_MESSAGE_HEADER) + 3, NULL);
  if (hr != S_OK) {
    WCHAR errorMessageBuffer[MAX_PATH];
    swprintf_s(errorMessageBuffer, L"FilterGetMessage failed 0x%08x", hr);
    Logger::log(LogLevelError, errorMessageBuffer);
    CloseHandle(hComPort);
    return ERROR_GEN_FAILURE;
  }

  if (replyBuffer[sizeof(FILTER_MESSAGE_HEADER)] == 'o') {
    Logger::log(LogLevelInfo, L"Connection to the driver established");
  } else {
    WCHAR errorMessageBuffer[MAX_PATH];
    swprintf_s(errorMessageBuffer, L"Connection failed");
    Logger::log(LogLevelError, errorMessageBuffer);
    CloseHandle(hComPort);
    return ERROR_GEN_FAILURE;
  }

  CopyMemory(&replyHeader, replyBuffer, sizeof(FILTER_REPLY_HEADER));
  replyHeader.Status = 0;

  return ERROR_SUCCESS;
}

DWORD COM::sendAndGetReply(const PBYTE sendMessage,
                           const DWORD& sendMessageSize, PBYTE replyMessage,
                           DWORD& replyMessageSize) {
  DWORD dwBytesReturned = 0;

  BYTE sendBuffer[sizeof(FILTER_MESSAGE_HEADER) + MAX_PATH];
  BYTE replyBuffer[sizeof(FILTER_MESSAGE_HEADER) + MAX_PATH];
  CopyMemory(sendBuffer, &replyHeader, sizeof(FILTER_REPLY_HEADER));
  CopyMemory(sendBuffer + sizeof(FILTER_REPLY_HEADER), sendMessage,
             sendMessageSize);
  HRESULT hr =
      FilterReplyMessage(hComPort, (PFILTER_REPLY_HEADER)sendBuffer,
                         sizeof(FILTER_REPLY_HEADER) + sendMessageSize);
  if (hr != S_OK) {
    WCHAR errorMessageBuffer[MAX_PATH];
    swprintf_s(errorMessageBuffer, L"FilterReplyMessage failed 0x%08x", hr);
    Logger::log(LogLevelError, errorMessageBuffer);
    CloseHandle(hComPort);
    return ERROR_GEN_FAILURE;
  }

  ZeroMemory(sendBuffer, sizeof(sendBuffer));
  ZeroMemory(replyBuffer, sizeof(replyBuffer));
  hr = FilterGetMessage(hComPort, (PFILTER_MESSAGE_HEADER)replyBuffer,
                        sizeof(FILTER_MESSAGE_HEADER) + 3, NULL);
  if (hr != S_OK) {
    WCHAR errorMessageBuffer[MAX_PATH];
    swprintf_s(errorMessageBuffer, L"FilterGetMessage failed 0x%08x", hr);
    Logger::log(LogLevelError, errorMessageBuffer);
    CloseHandle(hComPort);
    return ERROR_GEN_FAILURE;
  }

  //for (int i = 0; i < sizeof(replyBuffer); ++i) {
  //  std::wcout << replyBuffer[i] << " ";
  //}

  replyMessageSize = 3;
  CopyMemory(replyMessage, replyBuffer + sizeof(FILTER_MESSAGE_HEADER),
             replyMessageSize);
  return ERROR_SUCCESS;
}

DWORD COM::disconnect() {
  CloseHandle(hComPort);
  Logger::log(LogLevelInfo, L"Closed communication port");
  return ERROR_SUCCESS;
}