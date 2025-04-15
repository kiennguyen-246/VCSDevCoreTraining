#include "Logger.hpp"

const std::wstring kDefaultLogDir = L"%USERPROFILE%\\Logs\\logs.log";
const std::wstring kLogLevelStrings[] = {
	L"Info", L"Warning", L"Error"
};

std::wstring Logger::logDir = kDefaultLogDir;

DWORD Logger::init() {
	CreateDirectory(resolveDir(L"%USERPROFILE%\\Logs").c_str(), NULL);
	return ERROR_SUCCESS;
}

DWORD Logger::setLogDir(const std::wstring& __logDir) {
	logDir = __logDir;
	return ERROR_SUCCESS;
}

DWORD Logger::log(const LogLevel& logLevel, const std::wstring& message) {
	SYSTEMTIME st;
	GetLocalTime(&st);
	WCHAR logMessageBuffer[1024];
	swprintf_s(logMessageBuffer, L"[%02d/%02d/%04d %02d:%02d:%02d][%ws] %ws\n",
		st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond,
		kLogLevelStrings[logLevel].c_str(), message.c_str());
	std::wstring logMessage(logMessageBuffer);

	std::wofstream fs(resolveDir(logDir), std::wofstream::app);
	if (fs.is_open()) {
		fs << logMessage;
		fs.close();
	}
	else {
		throw std::ios_base::failure("Failed to open file: " + wstringToString(logDir));
		return ERROR_FILE_NOT_FOUND;
	}
	return ERROR_SUCCESS;
}