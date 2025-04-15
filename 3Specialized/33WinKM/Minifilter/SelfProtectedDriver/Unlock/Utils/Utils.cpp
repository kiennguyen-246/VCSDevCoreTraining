#include "Utils.hpp"

std::wstring resolveDir(const std::wstring& inp) {
	WCHAR outputBuffer[MAX_PATH];
	ZeroMemory(&outputBuffer, sizeof(outputBuffer));

	ExpandEnvironmentStrings(inp.c_str(), outputBuffer, sizeof(outputBuffer));
	std::wstring out(outputBuffer);

	return out;
}

std::string wstringToString(const std::wstring& inp) {
	std::string ret;
	for (WCHAR wc : inp) {
		auto c = (CHAR)wc;
		ret.push_back(c);
	}
	return ret;
}