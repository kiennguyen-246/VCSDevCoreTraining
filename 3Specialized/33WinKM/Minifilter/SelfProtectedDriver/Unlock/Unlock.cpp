#include "UI/AppUI.hpp"

int main() {
  auto* pAppUI = new AppUI;
  auto res = pAppUI->display();
  delete pAppUI;
  return res;
}