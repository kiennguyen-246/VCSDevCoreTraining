#include "pmon/PmonController.hpp"

int main(int argc, char* argv[]) {
  auto pPmonControllerInstance = PmonController::getInstance();
  pPmonControllerInstance->run();

  return 0;
}