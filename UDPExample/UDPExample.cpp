#include <iostream>
#include <string>
#include <sstream>
#include <WinSock2.h>
#include <WS2tcpip.h> 
#include <Windows.h>
#include <vector>
#include <algorithm>

#pragma comment(lib, "Ws2_32.lib")

void setColor(int colorNumber) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, colorNumber);
}

std::string processMessage(const std::string& message, const std::vector<std::string>& forbiddenWords) {
    std::string processedMessage = message;
    for (const auto& word : forbiddenWords) {
        size_t pos = 0;
        std::string mask(word.length(), '*');
        while ((pos = processedMessage.find(word, pos)) != std::string::npos) {
            processedMessage.replace(pos, word.length(), mask);
            pos += mask.length();
        }
    }
    return processedMessage;
}

int main()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "Failed to initialize Winsock." << std::endl;
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket == INVALID_SOCKET)
    {
        std::cerr << "Failed to create server socket." << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(55555);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "Failed to bind server socket." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    char buffer[1024];
    sockaddr_in clientAddr;
    int clientAddrSize = sizeof(clientAddr);
    std::vector<std::string> forbiddenWords = { "end", "start", "error", "hello" };

    while (true)
    {
        int bytesReceived = recvfrom(serverSocket, buffer, sizeof(buffer), 0, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrSize);
        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "Error in receiving data." << std::endl;
        }
        else
        {
            buffer[bytesReceived] = '\0';

            std::istringstream iss(buffer);
            int colorNumber;
            std::string nickname;
            std::string message;

            iss >> colorNumber >> nickname;
            std::getline(iss, message);

            char ipv4Address[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(clientAddr.sin_addr), ipv4Address, INET_ADDRSTRLEN);

            std::string processedMessage = processMessage(message, forbiddenWords);

            setColor(colorNumber);
            std::cout << "[Message] '" << nickname << "' (" << ipv4Address << "): " << processedMessage << std::endl;
            setColor(7); // белый цвет
        }
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
