#include <unistd.h>

#include <iostream>

int main() {
  auto pid = fork();

  int x = 1;

  if (pid == 0) {
    std::cout << "Child process, PID = " << getpid() << "\n";
    x = 5;
  } else {
    std::cout << "Parent process, PID = " << getpid() << "\n";
    x = 6;
  }
  std::cout << getpid() << "\n";
  std::cout << x << "\n";
  std::cout << 1/0;
}