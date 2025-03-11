// PEReader.cpp

#include <Windows.h>

#include <cassert>
#include <format>
#include <fstream>
#include <iostream>
#include <vector>

struct PEInfoEntry {
  std::wstring wsEntryName;
  DWORD dwSize;
  DWORD dwSize64;
  union {
    DWORD dwValue;
    ULONGLONG ullValue;
    CHAR pcValue[8];
    IMAGE_DATA_DIRECTORY iddValue;
  };
};

DWORD getLittleEndianNum(BYTE pbBuffer[], DWORD dwSize) {
  DWORD ret = 0;
  for (DWORD i = 0; i < dwSize; i++) {
    ret += (pbBuffer[i] << (i << 3));
  }
  return ret;
}

std::wstring mdNumRow(PEInfoEntry infoEntries[], DWORD dwInfoEntryID,
                      BOOL bIsPE64) {
  auto dwSize = bIsPE64 ? infoEntries[dwInfoEntryID].dwSize64
                        : infoEntries[dwInfoEntryID].dwSize;
  if (!dwSize) {
    return L"";
  }
  switch (dwSize) {
    case 1:
      return std::format(L"|{}|{}|0x{:02x}|\n",
                         infoEntries[dwInfoEntryID].wsEntryName,
                         infoEntries[dwInfoEntryID].dwValue,
                         infoEntries[dwInfoEntryID].dwValue);
    case 2:
      return std::format(L"|{}|{}|0x{:04x}|\n",
                         infoEntries[dwInfoEntryID].wsEntryName,
                         infoEntries[dwInfoEntryID].dwValue,
                         infoEntries[dwInfoEntryID].dwValue);
    case 4:
      return std::format(L"|{}|{}|0x{:08x}|\n",
                         infoEntries[dwInfoEntryID].wsEntryName,
                         infoEntries[dwInfoEntryID].dwValue,
                         infoEntries[dwInfoEntryID].dwValue);
    case 8:
      return std::format(L"|{}|{}|0x{:016x}|\n",
                         infoEntries[dwInfoEntryID].wsEntryName,
                         infoEntries[dwInfoEntryID].ullValue,
                         infoEntries[dwInfoEntryID].ullValue);
    default:
      assert(FALSE);
      return L"";
  }
}

std::wstring mdIDDRow(PEInfoEntry infoEntries[], DWORD dwInfoEntryID,
                      BOOL bIsPE64) {
  auto dwSize = bIsPE64 ? infoEntries[dwInfoEntryID].dwSize64
                        : infoEntries[dwInfoEntryID].dwSize;
  if (!dwSize) {
    return L"";
  }
  switch (dwSize) {
    case 8:
      return std::format(L"|{}|+0x{:08x}|{}|\n",
                         infoEntries[dwInfoEntryID].wsEntryName,
                         infoEntries[dwInfoEntryID].iddValue.VirtualAddress,
                         infoEntries[dwInfoEntryID].iddValue.Size);
    default:
      assert(FALSE);
      return L"";
  }
}

BOOL charStringToWcharString(LPSTR pcSrc, DWORD pcSrcLen, LPWSTR pwcDes) {
  for (DWORD i = 0; i < pcSrcLen; i++) {
    pwcDes[i] = pcSrc[i];
  }
  return TRUE;
}

std::wstring mdSectionTableRow(std::vector<PEInfoEntry> sectionInfo) {
  WCHAR pwcSectionName[9] = L"";
  charStringToWcharString(sectionInfo[0].pcValue, 8, pwcSectionName);
  //std::wstring wsSectionName(pwcSectionName);
  //std::wcout << wsSectionName << "\n";
  return std::format(
      L"|{}|{}|+0x{:08x}|{}|0x{:08x}|0x{:08x}|0x{:08x}|{}|{}|0x{:08x}|\n",
      pwcSectionName, sectionInfo[1].dwValue, sectionInfo[2].dwValue,
      sectionInfo[3].dwValue, sectionInfo[4].dwValue, sectionInfo[5].dwValue,
      sectionInfo[6].dwValue, sectionInfo[7].dwValue, sectionInfo[8].dwValue,
      sectionInfo[9].dwValue);
}

int wmain(int argc, LPWSTR argv[]) {
    std::wcout << sizeof(PVOID);
  return 0;

  DWORD dwError = 0;
  HANDLE hFile;
  BYTE pbBuffer[MAX_PATH];
  DWORD dwBytesRead = 0;
  DWORD dwCurFileOffset = 0;
  DWORD dwBytesAboutToRead = 0;
  BOOL bIsPE64 = FALSE;

  PEInfoEntry fileHeaderInfoEntries[] = {
      {L"Machine", 2, 2, 0},
      {L"NumberOfSections", 2, 2, 0},
      {L"TimeDateStamp", 4, 4, 0},
      {L"PointerToSymbolTable", 4, 4, 0},
      {L"NumberOfSymbols", 4, 4, 0},
      {L"SizeOfOptionalHeader", 2, 2, 0},
      {L"Characteristics", 2, 2, 0},
      {L"Magic", 2, 2, 0},
      {L"MajorLinkerVersion", 1, 1, 0},
      {L"MinorLinkerVersion", 1, 1, 0},
      {L"SizeOfCode", 4, 4, 0},
      {L"SizeOfInitializedData", 4, 4, 0},
      {L"SizeOfUninitializedData", 4, 4, 0},
      {L"AddressOfEntryPoint", 4, 4, 0},
      {L"BaseOfCode", 4, 4, 0},
      {L"BaseOfData", 4, 0, 0},
      {L"ImageBase", 4, 8, 0},
      {L"SectionAlignment", 4, 4, 0},
      {L"FileAlignment", 4, 4, 0},
      {L"MajorOperatingSystemVersion", 2, 2, 0},
      {L"MinorOperatingSystemVersion", 2, 2, 0},
      {L"MajorImageVersion", 2, 2, 0},
      {L"MinorImageVersion", 2, 2, 0},
      {L"MajorSubsystemVersion", 2, 2, 0},
      {L"MinorSubsystemVersion", 2, 2, 0},
      {L"Win32VersionValue", 4, 4, 0},
      {L"SizeOfImage", 4, 4, 0},
      {L"SizeOfHeaders", 4, 4, 0},
      {L"CheckSum", 4, 4, 0},
      {L"Subsystem", 2, 2, 0},
      {L"DllCharacteristics", 2, 2, 0},
      {L"SizeOfStackReserve", 4, 8, 0},
      {L"SizeOfStackCommit", 4, 8, 0},
      {L"SizeOfHeapReserve", 4, 8, 0},
      {L"SizeOfHeapCommit", 4, 8, 0},
      {L"LoaderFlags", 4, 4, 0},
      {L"NumberOfRvaAndSizes", 4, 4, 0},
      {L"Export Table (.edata)", 8, 8, 0},
      {L"Import Table (.idata)", 8, 8, 0},
      {L"Resource Table (.rsrc)", 8, 8, 0},
      {L"Exception Table (.pdata)", 8, 8, 0},
      {L"Certificate Table", 8, 8, 0},
      {L"Base Relocation Table (.reloc)", 8, 8, 0},
      {L"Debug (.debug)", 8, 8, 0},
      {L"Architecture", 8, 8, 0},
      {L"Global Ptr", 8, 8, 0},
      {L"TLS Table (.tls)", 8, 8, 0},
      {L"Load Config Table", 8, 8, 0},
      {L"Bound Import", 8, 8, 0},
      {L"IAT", 8, 8, 0},
      {L"Delay Import Descriptor", 8, 8, 0},
      {L"CLR Runtime Header (.cormeta)", 8, 8, 0},
      {L"Reserved", 8, 8, 0},
  };
  PEInfoEntry sectionTableInfoEntriesPrototype[10] = {
      {L"Name", 8, 8, 0},
      {L"VirtualSize", 4, 4, 0},
      {L"VirtualAddress", 4, 4, 0},
      {L"SizeOfRawData", 4, 4, 0},
      {L"PointerToRawData", 4, 4, 0},
      {L"PointerToRelocations", 4, 4, 0},
      {L"PointerToLinenumbers", 4, 4, 0},
      {L"NumberOfRelocations", 2, 2, 0},
      {L"NumberOfLinenumbers", 2, 2, 0},
      {L"Characteristics", 4, 4, 0}};
  std::vector<std::vector<PEInfoEntry> > vSectionTableInfoEntries;

  if (argc < 2) {
    std::wcout << L"Provide a path to a PE file.\n";
    return 0;
  }

  hFile = CreateFile(argv[1], GENERIC_READ, FILE_SHARE_READ, NULL,
                     OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hFile == INVALID_HANDLE_VALUE) {
    dwError = GetLastError();
    std::wcout << std::format(L"CreateFile failed {}\n", dwError);
    return 0;
  }

  // DOS stub
  ZeroMemory(pbBuffer, sizeof(pbBuffer));
  if (!ReadFile(hFile, pbBuffer, 0x3c, &dwBytesRead, NULL)) {
    dwError = GetLastError();
    std::wcout << std::format(L"ReadFile failed {}\n", dwError);
    CloseHandle(hFile);
    return 0;
  }
  dwCurFileOffset += 0x3c;

  // Find the signature offset
  ZeroMemory(pbBuffer, sizeof(pbBuffer));
  if (!ReadFile(hFile, pbBuffer, 4, &dwBytesRead, NULL)) {
    dwError = GetLastError();
    std::wcout << std::format(L"ReadFile failed {}\n", dwError);
    CloseHandle(hFile);
    return 0;
  }
  dwCurFileOffset += 4;
  DWORD dwSignatureOffset = pbBuffer[0] + (pbBuffer[1] << 8) +
                            (pbBuffer[2] << 16) + (pbBuffer[3] << 24);

  // Continue DOS stub
  ZeroMemory(pbBuffer, sizeof(pbBuffer));
  if (!ReadFile(hFile, pbBuffer, dwSignatureOffset - dwCurFileOffset,
                &dwBytesRead, NULL)) {
    dwError = GetLastError();
    std::wcout << std::format(L"ReadFile failed {}\n", dwError);
    CloseHandle(hFile);
    return 0;
  }
  dwCurFileOffset = dwSignatureOffset;

  // PE Signature
  ZeroMemory(pbBuffer, sizeof(pbBuffer));
  if (!ReadFile(hFile, pbBuffer, 4, &dwBytesRead, NULL)) {
    dwError = GetLastError();
    std::wcout << std::format(L"ReadFile failed {}\n", dwError);
    CloseHandle(hFile);
    return 0;
  }
  dwCurFileOffset += 4;
  if (pbBuffer[0] != 'P' || pbBuffer[1] != 'E' || pbBuffer[2] != 0 ||
      pbBuffer[3] != 0) {
    std::wcout << std::format(L"The specified file does not have PE format\n");
    CloseHandle(hFile);
    return 0;
  }

  // File headers
  BOOL bOptionalDataDirectories = FALSE;
  for (int i = 0; i < sizeof(fileHeaderInfoEntries) / sizeof(PEInfoEntry);
       i++) {
    if (fileHeaderInfoEntries[i].wsEntryName == L"Export Table (.edata)") {
      bOptionalDataDirectories |= 1;
    }
    if (fileHeaderInfoEntries[i].wsEntryName == L"BaseOfData") {
      i = i;
    }
    if (!bIsPE64) {
      dwBytesAboutToRead = fileHeaderInfoEntries[i].dwSize;
    } else {
      dwBytesAboutToRead = fileHeaderInfoEntries[i].dwSize64;
    }

    ZeroMemory(pbBuffer, sizeof(pbBuffer));
    if (!ReadFile(hFile, pbBuffer, dwBytesAboutToRead, &dwBytesRead, NULL)) {
      dwError = GetLastError();
      std::wcout << std::format(L"ReadFile failed {}\n", dwError);
      CloseHandle(hFile);
      return 0;
    }
    dwCurFileOffset += dwBytesAboutToRead;

    if (!bOptionalDataDirectories) {
      if (dwBytesAboutToRead < 8) {
        fileHeaderInfoEntries[i].dwValue =
            getLittleEndianNum(pbBuffer, dwBytesAboutToRead);
      } else {
        fileHeaderInfoEntries[i].ullValue =
            getLittleEndianNum(pbBuffer, dwBytesAboutToRead);
      }
    } else {
      CopyMemory(&fileHeaderInfoEntries[i].iddValue, pbBuffer,
                 dwBytesAboutToRead);
    }

    if (fileHeaderInfoEntries[i].wsEntryName == L"Magic" &&
        fileHeaderInfoEntries[i].dwValue == 0x20b) {
      bIsPE64 |= 1;
    }
  }

  // Section table
  std::vector<PEInfoEntry> currentSectionInfoEntries(10);
  for (DWORD dwSectionID = 0; dwSectionID < fileHeaderInfoEntries[1].dwValue;
       dwSectionID++) {
    for (int i = 0; i < 10; i++) {
      currentSectionInfoEntries[i] = sectionTableInfoEntriesPrototype[i];
    }
    for (int i = 0; i < 10; i++) {
      dwBytesAboutToRead = currentSectionInfoEntries[i].dwSize;
      ZeroMemory(pbBuffer, sizeof(pbBuffer));
      if (!ReadFile(hFile, pbBuffer, dwBytesAboutToRead, &dwBytesRead, NULL)) {
        dwError = GetLastError();
        std::wcout << std::format(L"ReadFile failed {}\n", dwError);
        CloseHandle(hFile);
        return 0;
      }
      dwCurFileOffset += dwBytesAboutToRead;
      if (i == 0) {
        CopyMemory(currentSectionInfoEntries[i].pcValue, pbBuffer, 8);
      } else {
        currentSectionInfoEntries[i].dwValue =
            getLittleEndianNum(pbBuffer, dwBytesAboutToRead);
      }
    }
    vSectionTableInfoEntries.push_back(currentSectionInfoEntries);
  }

  if (!CloseHandle(hFile)) {
    dwError = GetLastError();
    std::wcout << std::format(L"CloseHandle failed {}\n", dwError);
    return 0;
  }

  DWORD dwCurrentEntry = 0;
  std::wofstream wfs(".\\report.md");
  if (wfs.is_open()) {
    wfs << std::format(L"# {}\n\n", argv[1]);
    wfs << std::format(
        L"This is the analyzed result of the PE file located at {}.\n\n",
        argv[1]);
    wfs << std::format(
        L"To interpret the result, please refer to the MSDN document "
        L"[here](https://learn.microsoft.com/en-us/windows/win32/debug/"
        L"pe-format).\n\n");
    wfs << std::format(L"## File Headers\n\n");
    wfs << std::format(L"### MS-DOS Stub\n\n");
    wfs << std::format(
        L"The file begins with MS-DOS stub. Within the MS-DOS stub, the offset "
        L"to "
        L"the file signature is specified at offset 0x3c, which is {} "
        L"(0x{:08x}).\n\n",
        dwSignatureOffset, dwSignatureOffset);

    wfs << std::format(L"### File Signature \n\n");
    wfs << std::format(L"PE\\0\\0\n\n");

    wfs << std::format(L"### COFF File Header\n\n");
    wfs << std::format(L"|Field|Value (Dec)|Value (Hex)| \n");
    wfs << std::format(L"|---|---|---| \n");
    for (int i = 0; i < 7; i++) {
      wfs << mdNumRow(fileHeaderInfoEntries, dwCurrentEntry, bIsPE64);
      ++dwCurrentEntry;
    }

    wfs << std::format(L"### Optional Header\n\n");
    wfs << std::format(L"#### Optional Header Standard Fields\n\n");
    wfs << std::format(L"|Field|Value (Dec)|Value (Hex)| \n");
    wfs << std::format(L"|---|---|---| \n");
    for (int i = 0; i < 9; i++) {
      wfs << mdNumRow(fileHeaderInfoEntries, dwCurrentEntry, bIsPE64);
      ++dwCurrentEntry;
    }

    wfs << std::format(L"#### Optional Header Windows-Specific Fields\n\n");
    wfs << std::format(L"|Field|Value (Dec)|Value (Hex)| \n");
    wfs << std::format(L"|---|---|---| \n");
    for (int i = 0; i < 21; i++) {
      wfs << mdNumRow(fileHeaderInfoEntries, dwCurrentEntry, bIsPE64);
      ++dwCurrentEntry;
    }

    wfs << std::format(L"#### Optional Header Data Directories\n\n");
    wfs << std::format(L"|Field|Address|Size| \n");
    wfs << std::format(L"|---|---|---| \n");
    for (int i = 0; i < 16; i++) {
      wfs << mdIDDRow(fileHeaderInfoEntries, dwCurrentEntry, bIsPE64);
      ++dwCurrentEntry;
    }

    wfs << std::format(L"## Section Table\n\n");
    wfs << std::format(
        L"|Name|VirtualSize|VirtualAddress|SizeOfRawData|PointerToRawData|PointerToRelocations"
        L"|PointerToLinenumbers|NumberOfRelocations|NumberOfLinenumbers|Characteristics| \n");
    wfs << std::format(L"|---|---|---|---|---|---|---|---|---|---| \n");
    for (auto &sectionEntries : vSectionTableInfoEntries) {
      wfs << mdSectionTableRow(sectionEntries);
      ++dwCurrentEntry;
    }
  } else {
    std::wcout << L"Failed to open output file.\n";
  }
  wfs.close();
  std::wcout << L"The operation completed successfully\n";

  return 0;
}