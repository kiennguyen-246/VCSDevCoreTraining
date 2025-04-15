#pragma once

#include "UI/BaseUI.hpp"
#include "Controllers/UnlockFileController.hpp"

class UnlockFileUI : public BaseUI {
private:
	UnlockFileController* pUnlockFileController;
public:
	UnlockFileUI();
	~UnlockFileUI();

	DWORD display();
};