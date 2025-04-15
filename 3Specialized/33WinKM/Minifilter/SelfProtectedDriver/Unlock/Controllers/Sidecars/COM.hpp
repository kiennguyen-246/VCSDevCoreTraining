#pragma once

#include <Windows.h>
#include <fltUser.h>

#include <string>

#include "Logger.hpp"

class COM {
 private:
  std::wstring portName;
  HANDLE hComPort;
  FILTER_REPLY_HEADER replyHeader;

 public:
  COM();
  ~COM();

  DWORD setPortName(const std::wstring& __portName);

  DWORD connect();

  DWORD sendAndGetReply(const PBYTE sendMessage, const DWORD& sendMessageSize,
                        PBYTE replyMessage, DWORD& replyMessageSize);

  DWORD disconnect();
};