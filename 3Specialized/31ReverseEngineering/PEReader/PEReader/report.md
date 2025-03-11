# C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe

This is the analyzed result of the PE file located at C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe.

To interpret the result, please refer to the MSDN document [here](https://learn.microsoft.com/en-us/windows/win32/debug/pe-format).

## File Headers

### MS-DOS Stub

The file begins with MS-DOS stub. Within the MS-DOS stub, the offset to the file signature is specified at offset 0x3c, which is 120 (0x00000078).

### File Signature 

PE\0\0

### COFF File Header

|Field|Value (Dec)|Value (Hex)| 
|---|---|---| 
|Machine|34404|0x8664|
|NumberOfSections|13|0x000d|
|TimeDateStamp|1738218854|0x679b1d66|
|PointerToSymbolTable|0|0x00000000|
|NumberOfSymbols|0|0x00000000|
|SizeOfOptionalHeader|240|0x00f0|
|Characteristics|34|0x0022|
### Optional Header

#### Optional Header Standard Fields

|Field|Value (Dec)|Value (Hex)| 
|---|---|---| 
|Magic|523|0x020b|
|MajorLinkerVersion|14|0x0e|
|MinorLinkerVersion|0|0x00|
|SizeOfCode|2591232|0x00278a00|
|SizeOfInitializedData|1315328|0x00141200|
|SizeOfUninitializedData|0|0x00000000|
|AddressOfEntryPoint|1299904|0x0013d5c0|
|BaseOfCode|4096|0x00001000|
#### Optional Header Windows-Specific Fields

|Field|Value (Dec)|Value (Hex)| 
|---|---|---| 
|ImageBase|1073741825|0x0000000040000001|
|SectionAlignment|4096|0x00001000|
|FileAlignment|512|0x00000200|
|MajorOperatingSystemVersion|10|0x000a|
|MinorOperatingSystemVersion|0|0x0000|
|MajorImageVersion|0|0x0000|
|MinorImageVersion|0|0x0000|
|MajorSubsystemVersion|10|0x000a|
|MinorSubsystemVersion|0|0x0000|
|Win32VersionValue|0|0x00000000|
|SizeOfImage|4005888|0x003d2000|
|SizeOfHeaders|1024|0x00000400|
|CheckSum|3962527|0x003c769f|
|Subsystem|2|0x0002|
|DllCharacteristics|49504|0xc160|
|SizeOfStackReserve|8388608|0x0000000000800000|
|SizeOfStackCommit|4096|0x0000000000001000|
|SizeOfHeapReserve|1048576|0x0000000000100000|
|SizeOfHeapCommit|4096|0x0000000000001000|
|LoaderFlags|0|0x00000000|
|NumberOfRvaAndSizes|16|0x00000010|
#### Optional Header Data Directories

|Field|Address|Size| 
|---|---|---| 
|Export Table (.edata)|+0x002d9e41|135|
|Import Table (.idata)|+0x002d9ec8|80|
|Resource Table (.rsrc)|+0x00324000|698832|
|Exception Table (.pdata)|+0x00307000|77628|
|Certificate Table|+0x003bb600|10280|
|Base Relocation Table (.reloc)|+0x003cf000|11336|
|Debug (.debug)|+0x002d6208|84|
|Architecture|+0x00000000|0|
|Global Ptr|+0x00000000|0|
|TLS Table (.tls)|+0x002d5f40|40|
|Load Config Table|+0x0027a0d0|320|
|Bound Import|+0x00000000|0|
|IAT|+0x002da708|2032|
|Delay Import Descriptor|+0x002d88e8|544|
|CLR Runtime Header (.cormeta)|+0x00000000|0|
|Reserved|+0x00000000|0|
## Section Table

|Name|VirtualSize|VirtualAddress|SizeOfRawData|PointerToRawData|PointerToRelocations|PointerToLinenumbers|NumberOfRelocations|NumberOfLinenumbers|Characteristics| 
|---|---|---|---|---|---|---|---|---|---| 
|.text|2590813|+0x00001000|2591232|0x00000400|0x00000000|0x00000000|0|0|0x60000020|
|.rdata|447940|+0x0027a000|448000|0x00278e00|0x00000000|0x00000000|0|0|0x40000040|
|.data|123992|+0x002e8000|64512|0x002e6400|0x00000000|0x00000000|0|0|0xc0000040|
|.pdata|77628|+0x00307000|77824|0x002f6000|0x00000000|0x00000000|0|0|0x40000040|
|.gxfg|12272|+0x0031a000|12288|0x00309000|0x00000000|0x00000000|0|0|0x40000040|
|.retplne|196|+0x0031d000|512|0x0030c000|0x00000000|0x00000000|0|0|0x00000000|
|.tls|585|+0x0031e000|1024|0x0030c200|0x00000000|0x00000000|0|0|0xc0000040|
|CPADinfo|56|+0x0031f000|512|0x0030c600|0x00000000|0x00000000|0|0|0xc0000040|
|LZMADEC|4593|+0x00320000|4608|0x0030c800|0x00000000|0x00000000|0|0|0x60000020|
|_RDATA|500|+0x00322000|512|0x0030da00|0x00000000|0x00000000|0|0|0x40000040|
|malloc_h|219|+0x00323000|512|0x0030dc00|0x00000000|0x00000000|0|0|0x60000020|
|.rsrc|698832|+0x00324000|698880|0x0030de00|0x00000000|0x00000000|0|0|0x40000040|
|.reloc|11336|+0x003cf000|11776|0x003b8800|0x00000000|0x00000000|0|0|0x42000040|
