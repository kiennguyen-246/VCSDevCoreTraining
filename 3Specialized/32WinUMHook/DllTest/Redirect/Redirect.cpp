#include "Redirect.h"

#include "pch.h"

int redirect(const wchar_t path[]) {
  FILE* pFile;
  int iErr = 0;
  iErr = _wfreopen_s(&pFile, path, L"w", stdout);
  return iErr;
}