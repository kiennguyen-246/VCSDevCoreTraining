#include <iostream>
#include <fstream>

#pragma once

#include <Windows.h>

#include "Utils/Utils.hpp"

enum LogLevel { LogLevelInfo, LogLevelWarning, LogLevelError };

extern const std::wstring kDefaultLogDir;
extern const std::wstring kLogLevelStrings[];

class Logger {
private:
	static std::wstring logDir;

public:
	static DWORD init();

	static DWORD setLogDir(const std::wstring& __logDir);

	static DWORD log(const LogLevel& logLevel, const std::wstring& message);
};