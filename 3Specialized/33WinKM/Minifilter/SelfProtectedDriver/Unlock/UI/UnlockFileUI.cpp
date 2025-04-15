#include "UnlockFileUI.hpp"

UnlockFileUI::UnlockFileUI() {
  try {
    pUnlockFileController = new UnlockFileController();
  } catch (std::ios_base::failure e) {
    throw e;
  }
}

UnlockFileUI::~UnlockFileUI() { delete pUnlockFileController; }

DWORD UnlockFileUI::display() {
  std::wcout << L"Enter password: ";

  std::wstring password;
  std::wcin >> password;

  DWORD result = pUnlockFileController->unlock(password);
  switch (result) {
    case ERROR_SUCCESS:
      std::wcout << "Operation successful.\n";
      break;
    case ERROR_ACCESS_DENIED:
      std::wcout << "Sorry, try again.\n";
      break;
    default:
      std::wcout << "Something went wrong.\n";
      break;
  }

  std::cin.get();

  return ERROR_SUCCESS;
}