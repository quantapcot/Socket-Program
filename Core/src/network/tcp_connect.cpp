#define _HAS_STD_BYTE 0
#include "network/tcp_connect.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <cstring>
using namespace std;

#pragma comment(lib, "Ws2_32.lib")

bool initNetwork()
{
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (result != 0)
    {
        cout << "Loi khoi tao mang, ma loi: " << result << endl;
        return false;
    }
    return true;
}

void cleanupNetwork()
{
    WSACleanup();
}

// ===== Cac ham thanh vien cua class TcpConnection =====

TcpConnection::TcpConnection()
{
    socketHandle = 0;
    isValid = false;
}

TcpConnection::TcpConnection(unsigned long long existingSocket)
{
    socketHandle = existingSocket;
    isValid = true;
}

TcpConnection::~TcpConnection()
{
    // Do TcpConnection được copy by value nhiều lần (khi pass vào thread, session),
    // việc gọi closeConnection() tự động ở đây sẽ làm socket bị đóng sớm.
    // Người dùng class này phải gọi closeConnection() thủ công khi thực sự xong.
}

bool TcpConnection::connectToServer(string ip, int port)
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
    serverAddr.sin_port = htons(port);

    int convertResult = inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr);
    if (convertResult <= 0)
    {
        cout << "Dia chi IP khong hop le: " << ip << endl;
        closesocket(newSocket);
        return false;
    }

    int connectResult = connect(newSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if (connectResult == SOCKET_ERROR)
    {
        cout << "Ket noi toi server that bai" << endl;
        closesocket(newSocket);
        return false;
    }

    // Luu socket vua tao vao bien thanh vien cua object nay
    socketHandle = (unsigned long long)newSocket;
    isValid = true;
    return true;
}

bool TcpConnection::sendLine(string text)
{
    if (!isValid)
    {
        cout << "Ket noi khong hop le, khong the gui" << endl;
        return false;
    }

    SOCKET realSocket = (SOCKET)socketHandle;

    int result = send(realSocket, text.c_str(), (int)text.length(), 0);

    if (result == SOCKET_ERROR)
    {
        cout << "Gui du lieu that bai" << endl;
        return false;
    }

    return true;
}

string TcpConnection::receiveLine()
{
    if (!isValid)
    {
        cout << "Ket noi khong hop le, khong the nhan" << endl;
        return "";
    }

    SOCKET realSocket = (SOCKET)socketHandle;
    string result = "";
    char oneChar;

    while (true)
    {
        int bytesRead = recv(realSocket, &oneChar, 1, 0);

        if (bytesRead <= 0)
        {
            // Client/server ben kia ngat ket noi, hoac co loi
            isValid = false;
            break;
        }

        if (oneChar == '\n')
        {
            break;
        }

        if (oneChar != '\r')
        {
            result = result + oneChar;
        }
    }

    return result;
}

bool TcpConnection::isConnected()
{
    return isValid;
}

string TcpConnection::getRemoteIP()
{
    if (!isValid) return "";
    SOCKET realSocket = (SOCKET)socketHandle;
    sockaddr_in remoteAddr;
    int addrLen = sizeof(remoteAddr);
    if (getpeername(realSocket, (sockaddr*)&remoteAddr, &addrLen) == 0) {
        char ipStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(remoteAddr.sin_addr), ipStr, INET_ADDRSTRLEN);
        return string(ipStr);
    }
    return "";
}

void TcpConnection::closeConnection()
{
    if (isValid)
    {
        SOCKET realSocket = (SOCKET)socketHandle;
        closesocket(realSocket);
        isValid = false;
    }
}