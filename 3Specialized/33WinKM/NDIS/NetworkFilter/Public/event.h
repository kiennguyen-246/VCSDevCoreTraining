#pragma once

typedef struct Event_ {
  union {
    struct Event_* pNextEvent;
    unsigned long long uiIsValid;
  }kernelHeader;
  unsigned int uiId;
  unsigned int uiFilteringLayerId;
  unsigned int uiLocalAddress;
  unsigned int uiLocalPort;
  unsigned int uiRemoteAddress;
  unsigned int uiRemotePort;
  unsigned int uiProtocolNumber;
  unsigned long long uiProcessId;
  unsigned int uiProcessPathLength;
  wchar_t awcProcessPath[260];
  unsigned int uiSourceInterfaceIndex;
  unsigned int uiDestInterfaceIndex;
} Event, *PEvent;