#include "AppController.hpp"

AppController::AppController() {
	Logger::init();
	Logger::setLogDir(L"%USERPROFILE%\\Logs\\unlock.log");
	Logger::log(LogLevelInfo, L"App started");
}

AppController::~AppController() {
	Logger::log(LogLevelInfo, L"App ended");
}