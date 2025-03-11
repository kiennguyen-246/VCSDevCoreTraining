#ifndef TRANSPORT_MANAGER_HPP
#define TRANSPORT_MANAGER_HPP

#include <iostream>
#include <Windows.h>
#include <mutex>
#include <queue>

const int TM_MAX_BUFFER_SIZE = 1024;

class TransportManager {
 private:
  std::mutex mtx;
  std::queue<std::wstring> qwsData;
 public:
  TransportManager();
  ~TransportManager();

  HRESULT tmWriteData(LPBYTE pData, PULONG puiDataLength);

  HRESULT tmReadData(LPBYTE pData, PULONG puiDataLength);

  //bool peek(LPBYTE pOutBuffer);
};

#endif  // !TRANSPORT_MANAGER_HPP
