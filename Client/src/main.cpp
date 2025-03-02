#include <winsock2.h>
#include <ws2tcpip.h>

#include <iostream>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

SOCKET clientSocket;
sockaddr_in serverAddr;

void receiveMessages() {
  sockaddr_in senderAddr;
  int senderAddrSize = sizeof(senderAddr);

  if (clientSocket == INVALID_SOCKET) {
    std::cerr << "Error: clientSocket is INVALID_SOCKET\n";
    return;
  }

  while (true) {
    char buffer[1024];
    senderAddrSize = sizeof(senderAddr);

    int recvLen = recvfrom(clientSocket, buffer, sizeof(buffer) - 1, 0,
                           (sockaddr*)&senderAddr, &senderAddrSize);

    if (recvLen == SOCKET_ERROR) {
      int errorCode = WSAGetLastError();
      std::cerr << "recvfrom failed with error: " << errorCode << "\n";
      break;
    }

    buffer[recvLen] = '\0';
    char ipStr[INET_ADDRSTRLEN];
    InetNtopA(AF_INET, &senderAddr.sin_addr, ipStr,
              INET_ADDRSTRLEN); 
    std::cout << "Received from " << ipStr << ":" << ntohs(senderAddr.sin_port)
              << " - " << buffer << "\n";
  }
}

int main() {
  WSADATA wsaData;

  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    std::cerr << "WSAStartup failed: " << WSAGetLastError() << "\n";
    return 1;
  }

  clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (clientSocket == INVALID_SOCKET) {
    std::cerr << "Socket creation failed: " << WSAGetLastError() << "\n";
    WSACleanup();
    return 1;
  }

  sockaddr_in clientAddr;
  clientAddr.sin_family = AF_INET;
  clientAddr.sin_port = htons(0);
  clientAddr.sin_addr.s_addr = INADDR_ANY;

  if (bind(clientSocket, (sockaddr*)&clientAddr, sizeof(clientAddr)) ==
      SOCKET_ERROR) {
    std::cerr << "Bind failed: " << WSAGetLastError() << "\n";
    closesocket(clientSocket);
    WSACleanup();
    return 1;
  }

  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(8888);
  inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

  std::thread recvThread(receiveMessages);
  recvThread.detach();

  while (true) {
    std::string message;
    std::getline(std::cin, message);

    if (message == "exit") break;

    int sendLen = sendto(clientSocket, message.c_str(), message.size(), 0,
                         (sockaddr*)&serverAddr, sizeof(serverAddr));

    if (sendLen == SOCKET_ERROR) {
      std::cerr << "sendto failed: " << WSAGetLastError() << "\n";
    }
  }

  closesocket(clientSocket);
  WSACleanup();
  return 0;
}