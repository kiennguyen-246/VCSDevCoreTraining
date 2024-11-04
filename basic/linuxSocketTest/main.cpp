#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

int main(int argc, char* argv[]) {
  int iClientSocket = socket(AF_INET, SOCK_STREAM, 0);

  sockaddr_in serverAddress;
  serverAddress.sin_addr.s_addr = inet_addr("192.168.88.133");
  serverAddress.sin_port = htons(55555);
  serverAddress.sin_family = AF_INET;

  if (connect(iClientSocket, (sockaddr*)&serverAddress,
              sizeof(serverAddress)) == -1) {
    std::cout << std::format("Connect error {}\n", errno);
    return errno;
  }

  for (int i = 0; i < 10; i++) {
    char pcBuffer[1024];
    memset(pcBuffer, 0, sizeof(pcBuffer));
    recv(iClientSocket, pcBuffer, sizeof(pcBuffer), 0);
    std::cout << "They said: " << std::string(pcBuffer) << "\n";

    std::string sMsg = "I'm fine, thanks";
    send(iClientSocket, sMsg.c_str(), sMsg.length(), 0);
    sleep(10);
  }

  close(iClientSocket);

  return 0;
}