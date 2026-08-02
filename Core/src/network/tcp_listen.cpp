#define _HAS_STD_BYTE 0
#include "network/tcp_listen.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <cstring>
using namespace std;

TcpServer::TcpServer()
{
    serverSocketHandle = 0;
    isRunning = false;
}

TcpServer::~TcpServer()
{
    if (isRunning)
    {
        stopListening();
    }
}

bool TcpServer::startListening(int port)
{
    SOCKET newSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (newSocket == INVALID_SOCKET)
    {
        cout << "Tao socket that bai" << endl;
        return false;
    }

    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    int bindResult = bind(newSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if (bindResult == SOCKET_ERROR)
    {
        cout << "Bind that bai, port co the dang bi chiem dung" << endl;
        closesocket(newSocket);
        return false;
    }

    int listenResult = listen(newSocket, 5);
    if (listenResult == SOCKET_ERROR)
    {
        cout << "Listen that bai" << endl;
        closesocket(newSocket);
        return false;
    }

    serverSocketHandle = (unsigned long long)newSocket;
    isRunning = true;
    return true;
}

TcpConnection TcpServer::acceptClient()
{
    SOCKET realServerSocket = (SOCKET)serverSocketHandle;

    // Dung im o day cho toi khi co client ket noi toi
    SOCKET clientSocket = accept(realServerSocket, NULL, NULL);

    if (clientSocket == INVALID_SOCKET)
    {
        cout << "Accept that bai" << endl;
        return TcpConnection();  // tra ve 1 connection rong, khong hop le
    }

    // Tao 1 object TcpConnection moi, gan socket vua nhan duoc vao no
    return TcpConnection((unsigned long long)clientSocket);
}

void TcpServer::stopListening()
{
    if (isRunning)
    {
        SOCKET realSocket = (SOCKET)serverSocketHandle;
        closesocket(realSocket);
        isRunning = false;
    }
}