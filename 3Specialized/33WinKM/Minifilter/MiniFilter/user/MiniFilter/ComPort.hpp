#ifndef COM_PORT_HPP
#define COM_PORT_HPP

#include <Windows.h>
#include <fltUser.h>

#include <vector>

#include "public/public.h"
#include "utils.hpp"

typedef struct _COM_MESSAGE {
  FILTER_MESSAGE_HEADER header;
  MFLT_EVENT_RECORD eventRecord;
} COM_MESSAGE, *PCOM_MESSAGE;

class ComPort {
 public:
  ComPort();

  ~ComPort();

  HRESULT connectToKernelNode(std::wstring sPortName);

  HRESULT getRecord(PMFLT_EVENT_RECORD pEventRecord);

  // HRESULT getRecord(std::vector<MFLT_EVENT_RECORD>* pvEventRecord);

  HRESULT disconnectFromKernelMode();

 private:
  std::wstring wsComPortName;

  HANDLE hComPort;
};

#endif