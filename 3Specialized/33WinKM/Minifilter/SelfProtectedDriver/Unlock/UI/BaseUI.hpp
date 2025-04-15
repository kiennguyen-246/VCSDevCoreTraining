#pragma once

#include <iostream>
#include <future>
#include <Windows.h>

#include "Utils/Utils.hpp"
#include "Controllers/Sidecars/Logger.hpp"

class BaseUI {
protected:
	Logger* pLogger;
public:
	BaseUI();
	~BaseUI();

	virtual DWORD display() = 0;
};