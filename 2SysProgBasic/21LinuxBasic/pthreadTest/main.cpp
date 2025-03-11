#include <pthread.h>

#include <future>
#include <iostream>

void* printTID(void* args) {
  sleep(4);
  std::cout << pthread_self() << "\n";
  return (void*)0;
}

int main() {
  std::cout << pthread_self() << "\n";
  execve("./main2", NULL, NULL);

  //   auto fThread1 = std::async([&]() -> void {
  //     sleep(100);
  //     std::cout << pthread_self() << "\n";
  //   });
  // fThread1.get();

  return 0;
}