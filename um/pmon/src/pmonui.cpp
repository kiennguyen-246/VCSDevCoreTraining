#include "pmonui/PmonUIController.hpp"

int main(int argc, char* argv[]) {
  auto pPmonUIControllerInstance = PmonUIController::getInstance();
  pPmonUIControllerInstance->run();
  return 0;
}