#include "UnlockFileController.hpp"

const WCHAR kCOMPortName[] = L"\\UnlockCOM";

UnlockFileController::UnlockFileController() {
  pCOMPort = new COM();
  pCOMPort->setPortName(kCOMPortName);
  DWORD res = pCOMPort->connect();
  if (res != ERROR_SUCCESS) {
    throw std::ios_base::failure("Failed to connect to the driver");
  }
}

UnlockFileController::~UnlockFileController() {
  pCOMPort->disconnect();
  delete pCOMPort;
}

DWORD UnlockFileController::unlock(const std::wstring& password) {
  Logger::log(LogLevelInfo, L"Attempt unlocking\n");

  BYTE replyBuffer[MAX_PATH];
  ZeroMemory(replyBuffer, sizeof(replyBuffer));
  DWORD replyBufferSize = 0;
  DWORD res = pCOMPort->sendAndGetReply((PBYTE)password.c_str(),
                                        password.length() * sizeof(WCHAR),
                                        (PBYTE)replyBuffer, replyBufferSize);
  if (res != ERROR_SUCCESS) {
    return ERROR_GEN_FAILURE;
  }
  /*for (int i = 0; i < sizeof(replyBuffer); ++i) {
    std::wcout << replyBuffer[i] << " ";
  }*/
  if (replyBuffer[0] == 'o') {
    Logger::log(LogLevelInfo, L"Password is correct. Unlocking successful.\n");
    return ERROR_SUCCESS;
  } else {
    Logger::log(LogLevelInfo, L"Password is incorrect. Unlocking failed.\n");
    return ERROR_ACCESS_DENIED;
  }

  Logger::log(LogLevelInfo, L"Done attempt unlocking\n");
  return ERROR_GEN_FAILURE;
}