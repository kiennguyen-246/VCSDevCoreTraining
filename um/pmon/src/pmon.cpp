#include "pmon/PmonController.hpp"

int main(int argc, char* argv[]) {
  /*auto pPmonControllerInstance = PmonController::getInstance();
  pPmonControllerInstance->run();*/

  // Process p(660);
  // for (int i = 0; i < 1000; i++) {
  //   p.updateInfo();
  //   std::cout << p.getCpu() << " " << p.getRam() << " " << p.getDisk() <<
  //   "\n"; Sleep(3000);
  // }

  auto pInstance = PmonController::getInstance();
  pInstance->run();

  return 0;
}