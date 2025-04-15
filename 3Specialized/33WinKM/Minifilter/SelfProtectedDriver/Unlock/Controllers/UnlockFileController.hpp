#pragma once

#include "BaseController.hpp"
#include "Sidecars/COM.hpp"

extern const WCHAR kCOMPortName[];

class UnlockFileController : public BaseController {
 private:
  COM* pCOMPort;

 public:
  UnlockFileController();
  ~UnlockFileController();

  DWORD unlock(const std::wstring& password);
};