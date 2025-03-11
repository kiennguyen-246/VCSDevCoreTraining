#include <fstream>
#include <iostream>

int main() {
  std::ofstream fo;
  fo.open("test1.log");
  fo << "\033[1;31mHello \033[0mworld";
  fo.close();
}