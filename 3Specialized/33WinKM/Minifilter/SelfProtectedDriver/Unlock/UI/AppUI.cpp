#include "AppUI.hpp"

AppUI::AppUI() { pAppController = new AppController(); }

AppUI::~AppUI() { delete pAppController; }

DWORD AppUI::display() {
  DWORD res = ERROR_SUCCESS;
  try {
    auto* pUnlockFileUI = new UnlockFileUI;
    res = pUnlockFileUI->display();
    delete pUnlockFileUI;
  } catch (std::ios_base::failure e) {
    std::wcout << "Failed to connect to the driver";
    return ERROR_GEN_FAILURE;
  }

  return res;
}