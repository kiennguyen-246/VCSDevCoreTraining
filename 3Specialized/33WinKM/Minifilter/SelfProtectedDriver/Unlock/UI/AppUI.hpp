#pragma once

#include "UI/BaseUI.hpp"
#include "UI/UnlockFileUI.hpp"
#include "Controllers/AppController.hpp"
#include "Controllers/Sidecars/Logger.hpp"

class AppUI : public BaseUI {
private:
	AppController* pAppController;
public:
	AppUI();
	~AppUI();

	DWORD display();
};