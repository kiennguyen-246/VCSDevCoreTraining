#include <cassert>
#include <fstream>
#include <future>
#include <iostream>

#include "FilterUser.hpp"
#include "utils.hpp"

const WCHAR FILTER_NAME[] = L"MiniFilter";
const WCHAR FILTER_DRIVER_PATH[] = L"";
const WCHAR FILTER_COM_PORT_NAME[] = L"\\MiniFilterPort";
const WCHAR EVENT_LOG_FILE[] = L".\\events.log";
const WCHAR ERROR_LOG_FILE[] = L".\\error.log";
const WCHAR SERVER_HOST[] = L"192.168.0.104";
//const WCHAR SERVER_HOST[] = L"192.168.150.129";
const WCHAR SERVER_PORT[] = L"55555";
const WCHAR INSTRUCTIONS[] =
    L"Type:\n"
    L"\t- 'install' to install the filter,\n"
    L"\t- 'help' to consult instructions,\n "
    L"\t- 'start' to start filtering,\n "
    L"\t- 'stop' to stop filtering\n"
    L"\t- 'quit' to exit\n";

int wmain(int argc, LPWSTR argv[]) {
  HRESULT hr = S_OK;

  WebSocketClient wsc;

  wsc.init();
  wsc.handshake(SERVER_HOST, SERVER_PORT);
  wsc.queueMsg(L"24062004");
  wsc.queueMsg(L"06122004");

  getchar();
  wsc.setShouldStop();

  return 0;

  FILE* stream;
  _wfreopen_s(&stream, ERROR_LOG_FILE, L"w", stderr);

  FilterUser fuMfltUser(FILTER_NAME, FILTER_COM_PORT_NAME);
  std::future<HRESULT> fMainRoutine;
  bool bHasStarted = false;

  system("cls");

  wprintf(L"MINIFILTER\n-----------------------------\n\n");
  wprintf(INSTRUCTIONS);

  // ULONG uiFileVersionInfoBlockSize = GetFileVersionInfoSize(
  //     L"C:\\Program Files\\Microsoft Visual "
  //    L"Studio\\2022\\Community\\Common7\\IDE\\devenv.exe",
  //     NULL);
  // struct LANGCODEPAGE {
  //   WORD wLanguage;
  //   WORD wCodePage;
  // }* lpLangCodePage;
  // ULONG uiLangCodePageSize;
  // LPWSTR pwcFileVersionInfoEntryResult = NULL;
  // ULONG uiFileVersionInfoEntryResultSize = 0;
  // if (uiFileVersionInfoBlockSize != 0) {
  //   LPVOID lpFileVersionInfoBlock = malloc(uiFileVersionInfoBlockSize);
  //   if (GetFileVersionInfo(L"C:\\Program Files\\Microsoft Visual "
  //                          L"Studio\\2022\\Community\\Common7\\IDE\\devenv.exe",
  //                          0,
  //                          uiFileVersionInfoBlockSize,
  //                          lpFileVersionInfoBlock)) {
  //     VerQueryValue(lpFileVersionInfoBlock, L"\\VarFileInfo\\Translation",
  //                   (LPVOID*)&lpLangCodePage, (PUINT)&uiLangCodePageSize);
  //     BOOLEAN bHasCompanyName = FALSE;
  //     for (int i = 0; i < uiLangCodePageSize / sizeof(LANGCODEPAGE); i++) {
  //       wprintf(std::format(L"\\StringFileInfo\\{:04x}{:04x}\\ProductName",
  //                           lpLangCodePage[i].wLanguage,
  //                           lpLangCodePage[i].wCodePage)
  //                   .c_str());
  //       if (VerQueryValue(
  //               lpFileVersionInfoBlock,
  //               std::format(L"\\StringFileInfo\\{:04x}{:04x}\\ProductName",
  //                           lpLangCodePage[i].wLanguage,
  //                           lpLangCodePage[i].wCodePage)
  //                   .c_str(),
  //               (LPVOID*)&pwcFileVersionInfoEntryResult,
  //               (PUINT)&uiFileVersionInfoEntryResultSize)) {
  //         if (uiFileVersionInfoEntryResultSize != 0) {
  //           wprintf(L"Company name is %ws\n", pwcFileVersionInfoEntryResult);
  //           bHasCompanyName = TRUE;
  //           break;
  //         }
  //       }
  //     }
  //     if (!bHasCompanyName) {
  //       //jsObj.addSingleObj(L"imageFileCompanyName", L"Unknown");
  //       wprintf(L"Company name is unknown");
  //     }
  //   } else {

  //  }
  //  free(lpFileVersionInfoBlock);
  //} else {
  //
  //}

  while (1) {
    wprintf(L"MiniFilter>> ");
    std::wstring wsCommand;
    std::wcin >> wsCommand;
    if (wsCommand == L"install") {
      wprintf(L"Command currently disabled\n");
    } else if (wsCommand == L"start") {
      hr = fuMfltUser.loadFilter();
      if (hr) {
        continue;
      }

      hr = fuMfltUser.connectToServer(SERVER_HOST, SERVER_PORT);

      // hr = fuMfltUser.attachFilterToVolume(L"E:");
      if (hr) {
        continue;
      }

      fMainRoutine = std::async(std::launch::async, &FilterUser::doMainRoutine,
                                &fuMfltUser);

      bHasStarted = true;
      wprintf(L"Successful\n");
    } else if (wsCommand == L"stop") {
      if (!bHasStarted) {
        wprintf(L"The filter has not started\n");
        continue;
      }
      fuMfltUser.setShouldStop();
      hr = fMainRoutine.get();
      if (FAILED(hr)) {
        wprintf(L"An error occured while filtering. Error 0x%08x\n", hr);
        // continue;
      }

      // hr = fuMfltUser.detachFilterFromVolume(L"E:");
      // if (hr) {
      //   continue;
      // }

      hr = fuMfltUser.unloadFilter();
      if (hr) {
        continue;
      }

      hr = fuMfltUser.disconnectFromServer();
      if (hr) {
        continue;
      }

      bHasStarted = false;

      wprintf(L"Successful\n");
    } else if (wsCommand == L"quit") {
      if (bHasStarted) {
        wprintf(L"Please stop the filter before exiting\n");
        continue;
      }
      break;
    } else if (wsCommand == L"help") {
      wprintf(INSTRUCTIONS);
    } else {
      wprintf(L"Invalid command. Type 'help' to consult instructions.\n");
    }
  }

  return 0;
}