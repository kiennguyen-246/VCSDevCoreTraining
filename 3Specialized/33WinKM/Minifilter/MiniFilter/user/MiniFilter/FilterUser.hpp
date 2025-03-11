#ifndef MFLT_USER_HPP
#define MFLT_USER_HPP

#include <format>
#include <fstream>
#include <future>
#include <map>

#include "WebSocketClient.hpp"
#include "ComPort.hpp"
#include "JSONObj.hpp"
#include "utils.hpp"

const WCHAR DEFAULT_LOG_FILE_PATH[] = L".\\events.log";
const int MAX_SAVED_LOG_LENGTH = 1024;
const int MD5_HASH_LENGTH = 16;
const int SHA1_HASH_LENGTH = 20;
const int SHA256_HASH_LENGTH = 32;

typedef struct _LANGUAGE_CODE_PAGE_STRUCT {
  WORD wLanguage;
  WORD wCodePage;
} LANGUAGE_CODE_PAGE_STRUCT, *PLANGUAGE_CODE_PAGE_STRUCT;

class FilterUser {
 public:
  FilterUser(std::wstring ws__FilterName, std::wstring ws__ComPortName);
  ~FilterUser();

  HRESULT loadFilter();

  HRESULT unloadFilter();

  HRESULT connectToServer(std::wstring wsHost, std::wstring wsPort);

  HRESULT disconnectFromServer();

  HRESULT attachFilterToVolume(std::wstring wsVolumeName);

  HRESULT detachFilterFromVolume(std::wstring wsVolumeName);

  HRESULT doMainRoutine();

  HRESULT setShouldStop();

 private:
  ComPort cp;

  std::wstring wsFilterName;

  std::wstring wsComPortName;

  std::wstring wsLogFilePath;

  std::wofstream wfsLog;

  std::wstring wsSavedLog;

  std::unordered_map<std::wstring, std::wstring> umDosDevices;

  std::mutex mtx;

  bool bIsFilterLoaded;

  bool bIsComPortConnected;

  volatile bool bShouldStop;

  ULONG uiEventId;

  WebSocketClient wsc;

  HRESULT setPrivilege(HANDLE hToken, LPCWSTR pwcPrivilege,
                       BOOL bIsPrivilegeEnabled);

  HRESULT composeEventLog(PMFLT_EVENT_RECORD pEventRecord,
                          std::wstring* pwsMsg);

  HRESULT logFileEvent(PMFLT_EVENT_RECORD pEventRecord, JSONObj* pJsObj);

  HRESULT logCreateProcessEvent(PMFLT_EVENT_RECORD pEventRecord,
                                JSONObj* pJsObj);

  std::wstring getFileVersionInfoEntry(
      PVOID lpFileVersionInfoBlock, PLANGUAGE_CODE_PAGE_STRUCT lpLangCodePage,
      ULONG uiLangCodePageSize, std::wstring wsQueryEntry);

  std::wstring getFileHash(std::wstring wsFileName, ALG_ID hashAlg);
};

#endif