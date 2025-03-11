#include <Windows.h>

#include <iostream>

typedef struct _IMAGE_BASE_RELOCATION_BLOCK_ENTRY {
  WORD wOffset : 12;
  WORD wType : 4;
} IMAGE_BASE_RELOCATION_BLOCK_ENTRY, *PIMAGE_BASE_RELOCATION_BLOCK_ENTRY;

typedef BOOL(WINAPI* FPDLLMAIN)(HINSTANCE, DWORD, LPVOID);

typedef struct _DLLMAIN_INFO {
  FPDLLMAIN fpDllMain;
  HINSTANCE hInstance;
  DWORD dwReason;
  LPVOID lpReserved;
} DLLMAIN_INFO, *PDLLMAIN_INFO;

DWORD WINAPI remoteDllMain(PDLLMAIN_INFO pDllMainInfo) {
  return pDllMainInfo->fpDllMain(pDllMainInfo->hInstance,
                                 pDllMainInfo->dwReason,
                                 pDllMainInfo->lpReserved);
}

typedef DWORD(WINAPI* FP_REMOTEDLLMAIN)(PDLLMAIN_INFO);

int wmain(DWORD argc, PWSTR argv[]) {
  if (argc < 2) {
    std::wcout << L"Usage: ReflectiveDllInjection <DLL Path>" << std::endl;
    return 1;
  }

  DWORD dwPid = GetCurrentProcessId();
  std::wcout << "PID: ";
  std::wcin >> dwPid;

  HINSTANCE hKernel32 = GetModuleHandle(TEXT("Kernel32.dll"));
  if (!hKernel32) {
    DWORD dwError = GetLastError();
    std::wcout << "GetModuleHandle failed " << dwError << "\n";
    return dwError;
  }

  HANDLE hRemoteProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);
  if (hRemoteProcess == NULL) {
    DWORD dwError = GetLastError();
    std::wcout << L"OpenProcess failed with error: " << dwError << std::endl;
    return 1;
  }

  // 1. Read the DLL content to a buffer
  HANDLE hDllFile =
      CreateFile(argv[1], GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
  if (hDllFile == INVALID_HANDLE_VALUE) {
    DWORD dwError = GetLastError();
    std::wcout << L"CreateFile failed with error: " << dwError << std::endl;
    return 1;
  }
  DWORD dwDllFileSize = GetFileSize(hDllFile, NULL);
  if (dwDllFileSize == INVALID_FILE_SIZE) {
    DWORD dwError = GetLastError();
    std::wcout << L"GetFileSize failed with error: " << dwError << std::endl;
    CloseHandle(hDllFile);
    return 1;
  }
  PVOID pDllContentBuffer = HeapAlloc(GetProcessHeap(), 0, dwDllFileSize);
  if (pDllContentBuffer == NULL) {
    DWORD dwError = GetLastError();
    std::wcout << L"HeapAlloc failed with error: " << dwError << std::endl;
    CloseHandle(hDllFile);
    return 1;
  }
  DWORD dwBytesRead = 0;
  BOOL bRet =
      ReadFile(hDllFile, pDllContentBuffer, dwDllFileSize, &dwBytesRead, NULL);
  if (!bRet) {
    DWORD dwError = GetLastError();
    std::wcout << L"ReadFile failed with error: " << dwError << std::endl;
    CloseHandle(hDllFile);
    HeapFree(GetProcessHeap(), 0, pDllContentBuffer);
    return 1;
  }
  CloseHandle(hDllFile);

  // PVOID pDllRemoteContentBuffer =
  //     VirtualAllocEx(hRemoteProcess, NULL, dwDllFileSize,
  //                    MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
  // WriteProcessMemory(hRemoteProcess, pDllRemoteContentBuffer,
  // pDllContentBuffer,
  //                    dwDllFileSize, NULL);

  // 2. Parse DLL content to get the SizeOfImage field
  PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pDllContentBuffer;
  PIMAGE_NT_HEADERS pNtHeader =
      (PIMAGE_NT_HEADERS)((PBYTE)pDllContentBuffer + pDosHeader->e_lfanew);
  DWORD dwSizeOfImage = pNtHeader->OptionalHeader.SizeOfImage;

  // 3. Allocate memory at BaseOfImage with the size of dwSizeOfImage
  ULONGLONG ullImageBase = pNtHeader->OptionalHeader.ImageBase;
  // PVOID pDllLoadBase =
  //     VirtualAlloc((PVOID)ullImageBase, dwSizeOfImage, MEM_COMMIT |
  //     MEM_RESERVE,
  //                  PAGE_EXECUTE_READWRITE);
  PVOID pDllLoadBase = NULL;
  while (pDllLoadBase == NULL) {
    pDllLoadBase = VirtualAllocEx(hRemoteProcess, (PVOID)((PBYTE)ullImageBase),
                                  dwSizeOfImage, MEM_COMMIT | MEM_RESERVE,
                                  PAGE_EXECUTE_READWRITE);
    if (pDllLoadBase == NULL) {
      DWORD dwError = GetLastError();
      if (dwError == ERROR_INVALID_ADDRESS) {
        ullImageBase += 0x10000;
        continue;
      }
      std::wcout << L"VirtualAllocEx failed with error: " << dwError
                 << std::endl;
      HeapFree(GetProcessHeap(), 0, pDllContentBuffer);
      return 1;
    }
  }

  // 4. Copy the DLL headers and PE sections to the memory space allocated in
  // the last step

  // 4.1. Copy the headers
  // CopyMemory(pDllLoadBase, pDllContentBuffer,
  //           pNtHeader->OptionalHeader.SizeOfHeaders);
  WriteProcessMemory(hRemoteProcess, pDllLoadBase, pDllContentBuffer,
                     pNtHeader->OptionalHeader.SizeOfHeaders, NULL);

  // 4.2. Copy the sections
  PIMAGE_SECTION_HEADER pCurrentSectionHeader = IMAGE_FIRST_SECTION(pNtHeader);
  for (DWORD i = 0; i < pNtHeader->FileHeader.NumberOfSections; ++i) {
    // CopyMemory(
    //     (PVOID)((PBYTE)pDllLoadBase + pCurrentSectionHeader->VirtualAddress),
    //     (PVOID)((PBYTE)pDllContentBuffer +
    //             pCurrentSectionHeader->PointerToRawData),
    //     pCurrentSectionHeader->SizeOfRawData);
    WriteProcessMemory(
        hRemoteProcess,
        (PVOID)((PBYTE)pDllLoadBase + pCurrentSectionHeader->VirtualAddress),
        (PVOID)((PBYTE)pDllContentBuffer +
                pCurrentSectionHeader->PointerToRawData),
        pCurrentSectionHeader->SizeOfRawData, NULL);
    ++pCurrentSectionHeader;
  }

  // 5. Perform image base relocations
  // 5.1. Get the delta
  ULONGLONG ullDelta =
      (ULONGLONG)pDllLoadBase - pNtHeader->OptionalHeader.ImageBase;

  // 5.2. Jump to the first relocation block
  PIMAGE_BASE_RELOCATION pRelocationTable =
      (PIMAGE_BASE_RELOCATION)((PBYTE)pDllLoadBase +
                               pNtHeader->OptionalHeader
                                   .DataDirectory
                                       [IMAGE_DIRECTORY_ENTRY_BASERELOC]
                                   .VirtualAddress);

  // 5.3. For each entries within, take the 12 lower bits, sum them with delta
  // to get the new address
  DWORD dwRelocationBytesProceeded = 0;
  DWORD dwRelocationTableSize =
      pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC]
          .Size;
  while (dwRelocationBytesProceeded < dwRelocationTableSize) {
    PIMAGE_BASE_RELOCATION pCurrentBlockHeader =
        (PIMAGE_BASE_RELOCATION)((PBYTE)pRelocationTable +
                                 dwRelocationBytesProceeded);

    IMAGE_BASE_RELOCATION currentBlockHeader;
    ReadProcessMemory(hRemoteProcess, pCurrentBlockHeader, &currentBlockHeader,
                      sizeof(IMAGE_BASE_RELOCATION), NULL);

    dwRelocationBytesProceeded += sizeof(IMAGE_BASE_RELOCATION);
    DWORD dwNumberOfEntries =
        (currentBlockHeader.SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) /
        sizeof(IMAGE_BASE_RELOCATION_BLOCK_ENTRY);
    for (DWORD i = 0; i < dwNumberOfEntries; ++i) {
      PIMAGE_BASE_RELOCATION_BLOCK_ENTRY pCurrentBlockEntry =
          (PIMAGE_BASE_RELOCATION_BLOCK_ENTRY)((PBYTE)pRelocationTable +
                                               dwRelocationBytesProceeded);

      IMAGE_BASE_RELOCATION_BLOCK_ENTRY currentBlockEntry;
      ReadProcessMemory(hRemoteProcess, pCurrentBlockEntry, &currentBlockEntry,
                        sizeof(IMAGE_BASE_RELOCATION_BLOCK_ENTRY), NULL);
      dwRelocationBytesProceeded += sizeof(IMAGE_BASE_RELOCATION_BLOCK_ENTRY);
      if (currentBlockEntry.wType == 0) {
        continue;
      }
      DWORD dwRelocationOffset =
          currentBlockHeader.VirtualAddress + currentBlockEntry.wOffset;
      PDWORD pdwAddressToPatch = 0;
      // ReadProcessMemory(GetCurrentProcess(),
      //                   (PVOID)((PBYTE)pDllLoadBase + dwRelocationOffset),
      //                   &pdwAddressToPatch, sizeof(PDWORD), NULL);
      ReadProcessMemory(hRemoteProcess,
                        (PVOID)((PBYTE)pDllLoadBase + dwRelocationOffset),
                        &pdwAddressToPatch, sizeof(PDWORD), NULL);
      pdwAddressToPatch = (PDWORD)((ULONGLONG)pdwAddressToPatch + ullDelta);
      // CopyMemory((PVOID)((PBYTE)pDllLoadBase + dwRelocationOffset),
      //            &pdwAddressToPatch, sizeof(PDWORD));
      WriteProcessMemory(hRemoteProcess,
                         (PVOID)((PBYTE)pDllLoadBase + dwRelocationOffset),
                         &pdwAddressToPatch, sizeof(PDWORD), NULL);
    }
  }

  // 6. Resolve the IAT
  // 6.1 Get the first import descriptor
  PIMAGE_IMPORT_DESCRIPTOR pImportDescriptor =
      (PIMAGE_IMPORT_DESCRIPTOR)((PBYTE)pDllLoadBase +
                                 pNtHeader->OptionalHeader
                                     .DataDirectory
                                         [IMAGE_DIRECTORY_ENTRY_IMPORT]
                                     .VirtualAddress);

  // 6.2. Iterate over import descriptors
  HMODULE hImportedLib = NULL;
  while (TRUE) {
    IMAGE_IMPORT_DESCRIPTOR currentImportDescriptor;
    ReadProcessMemory(hRemoteProcess, pImportDescriptor,
                      &currentImportDescriptor, sizeof(IMAGE_IMPORT_DESCRIPTOR),
                      NULL);
    if (currentImportDescriptor.Name == 0) {
      break;
    }

    // 6.2.1. Load the imported library
    PCHAR pcImportedLibName =
        (PCHAR)((PBYTE)pDllLoadBase + currentImportDescriptor.Name);
    CHAR pcCopiedImportedLibName[MAX_PATH];
    ReadProcessMemory(hRemoteProcess, pcImportedLibName,
                      pcCopiedImportedLibName, MAX_PATH, NULL);

    hImportedLib = LoadLibraryA(pcCopiedImportedLibName);
    if (hImportedLib == NULL) {
      DWORD dwError = GetLastError();
      std::wcout << L"LoadLibraryA for library " << pcCopiedImportedLibName
                 << " failed with error: " << dwError << std::endl;
      continue;
    }

     FARPROC fpLoadLibrary = GetProcAddress(hKernel32, "LoadLibraryA");
     DWORD dwRemoteTid = 0;
     HANDLE hRemoteThread = CreateRemoteThread(
         hRemoteProcess, NULL, 0, (LPTHREAD_START_ROUTINE)fpLoadLibrary,
         pcImportedLibName, 0, &dwRemoteTid);

    // 6.2.2. Get all thunks in the library
    PIMAGE_THUNK_DATA pThunkData =
        (PIMAGE_THUNK_DATA)((PBYTE)pDllLoadBase +
                            currentImportDescriptor.FirstThunk);

    // 6.2.3. For each thunk, resolve its address using GetProcAddress(), then
    // put it into the IAT
    while (TRUE) {
      IMAGE_THUNK_DATA currentThunkData;
      ReadProcessMemory(hRemoteProcess, pThunkData, &currentThunkData,
                        sizeof(IMAGE_THUNK_DATA), NULL);
      if (currentThunkData.u1.AddressOfData == 0) {
        break;
      }

      if (IMAGE_SNAP_BY_ORDINAL(currentThunkData.u1.Ordinal)) {
        PCHAR pcFunctionOrdinal =
            (PCHAR)IMAGE_ORDINAL(currentThunkData.u1.Ordinal);
        currentThunkData.u1.Function =
            (ULONGLONG)GetProcAddress(hImportedLib, pcFunctionOrdinal);
      } else {
        PIMAGE_IMPORT_BY_NAME pImportByName =
            (PIMAGE_IMPORT_BY_NAME)((PBYTE)pDllLoadBase +
                                    currentThunkData.u1.AddressOfData);
        CHAR pcCopiedFunctionName[MAX_PATH];
        ReadProcessMemory(hRemoteProcess, pImportByName->Name,
                          pcCopiedFunctionName, MAX_PATH, NULL);
        currentThunkData.u1.Function =
            (ULONGLONG)GetProcAddress(hImportedLib, pcCopiedFunctionName);
        // pThunkData->u1.Function =
        //     (ULONGLONG)GetProcAddress(hImportedLib, pImportByName->Name);
      }
      WriteProcessMemory(hRemoteProcess, pThunkData, &currentThunkData,
                         sizeof(IMAGE_THUNK_DATA), NULL);
      ++pThunkData;
    }
    ++pImportDescriptor;
  }

  // 7. Call the DLL's entry point
  FPDLLMAIN fpDllMain =
      (FPDLLMAIN)((PBYTE)pDllLoadBase +
                  pNtHeader->OptionalHeader.AddressOfEntryPoint);
   //fpDllMain((HINSTANCE)pDllLoadBase, DLL_PROCESS_ATTACH, NULL);

  CreateRemoteThread(hRemoteProcess, NULL, 0, (LPTHREAD_START_ROUTINE)fpDllMain,
                     (HINSTANCE)pDllLoadBase, 0, NULL);

  //PDLLMAIN_INFO pRemoteDllMainInfo = (PDLLMAIN_INFO)VirtualAllocEx(
  //    hRemoteProcess, NULL, sizeof(DLLMAIN_INFO), MEM_COMMIT, PAGE_READWRITE);
  //if (pRemoteDllMainInfo == NULL) {
  //  DWORD dwError = GetLastError();
  //  std::wcout << L"VirtualAllocEx failed with error: " << dwError << std::endl;
  //  HeapFree(GetProcessHeap(), 0, pDllContentBuffer);
  //  return 1;
  //}
  //DLLMAIN_INFO dllMainInfo = {fpDllMain, (HINSTANCE)pDllLoadBase,
  //                            DLL_PROCESS_ATTACH, NULL};
  //WriteProcessMemory(hRemoteProcess, pRemoteDllMainInfo, &dllMainInfo,
  //                   sizeof(DLLMAIN_INFO), NULL);

  //HANDLE hRemoteThread = CreateRemoteThread(
  //    hRemoteProcess, NULL, 0, (LPTHREAD_START_ROUTINE)remoteDllMain,
  //    pRemoteDllMainInfo, 0, NULL);
  //if (hRemoteThread == NULL) {
  //  DWORD dwError = GetLastError();
  //  std::wcout << L"CreateRemoteThread failed with error: " << dwError
  //             << std::endl;
  //  HeapFree(GetProcessHeap(), 0, pDllContentBuffer);
  //  return 1;
  //}
  //DWORD dwExitCode = 0;
  //GetExitCodeThread(hRemoteThread, &dwExitCode);
  //std::wcout << L"Remote thread exited with code: " << dwExitCode << std::endl;

  HeapFree(GetProcessHeap(), 0, pDllContentBuffer);
  // VirtualFreeEx(hRemoteProcess, pDllRemoteContentBuffer, 0, MEM_RELEASE);
  return 0;
}