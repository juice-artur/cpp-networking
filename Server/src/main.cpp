#include <winsock2.h>
#include <ws2tcpip.h>

#include <algorithm>
#include <iostream>
#include <vector>

#include "pqxx/pqxx"

#pragma comment(lib, "ws2_32.lib")

struct ClientInfo {
  sockaddr_in addr;
  int addrLen;
};

std::vector<ClientInfo> clients;

void addClient(sockaddr_in clientAddr, int addrLen) {
  for (const auto& client : clients) {
    if (client.addr.sin_addr.s_addr == clientAddr.sin_addr.s_addr &&
        client.addr.sin_port == clientAddr.sin_port) {
      return;
    }
  }
  clients.push_back({clientAddr, addrLen});
  char ipStr[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &clientAddr.sin_addr, ipStr, INET_ADDRSTRLEN);
  std::cout << "Reseived from " << ipStr << ":" << ntohs(clientAddr.sin_port)
            << " - " << ipStr << "\n";
}

void broadcastMessage(SOCKET serverSocket, char* message, int msgLen,
                      sockaddr_in senderAddr) {
  for (const auto& client : clients) {
    if (client.addr.sin_addr.s_addr != senderAddr.sin_addr.s_addr ||
        client.addr.sin_port != senderAddr.sin_port) {
      sendto(serverSocket, message, msgLen, 0, (sockaddr*)&client.addr,
             client.addrLen);
    }
  }
}

int main() {
  pqxx::connection conn(
      "host=localhost port=5432 dbname=db user=user password=pass");

  if (conn.is_open()) {
    std::cout << "Connected to database: " << conn.dbname() << std::endl;
    pqxx::work txn(conn);
    pqxx::result res = txn.exec("SELECT version();");
    txn.commit();

    // Вивід результату
    for (auto row : res) {
      std::cout << "PostgreSQL version: " << row[0].c_str() << std::endl;
    }
  }

  WSADATA wsaData;
  SOCKET serverSocket;
  sockaddr_in serverAddr, clientAddr;
  char buffer[1024];
  int clientAddrSize = sizeof(clientAddr);

  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    std::cerr << "WSAStartup failed\n";
    return 1;
  }

  serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (serverSocket == INVALID_SOCKET) {
    std::cerr << "Socket creation failed\n";
    WSACleanup();
    return 1;
  }

  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  serverAddr.sin_port = htons(8888);

  if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) ==
      SOCKET_ERROR) {
    std::cerr << "Bind failed\n";
    closesocket(serverSocket);
    WSACleanup();
    return 1;
  }

  std::cout << "UDP server port 8888...\n";

  while (true) {
    int recvLen = recvfrom(serverSocket, buffer, sizeof(buffer) - 1, 0,
                           (sockaddr*)&clientAddr, &clientAddrSize);
    if (recvLen == SOCKET_ERROR) {
      std::cerr << "recvfrom failed\n";
      break;
    }
    buffer[recvLen] = '\0';

    addClient(clientAddr, clientAddrSize);
    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, ipStr, INET_ADDRSTRLEN);
    std::cout << "reseived from" << ipStr << ":" << ntohs(clientAddr.sin_port)
              << " - " << buffer << "\n";

    broadcastMessage(serverSocket, buffer, recvLen, clientAddr);
  }

  closesocket(serverSocket);
  WSACleanup();
  return 0;
}
