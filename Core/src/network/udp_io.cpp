#include "network/udp_io.h"

#include <iostream>
#include <cstring>

UdpSocket::UdpSocket() {
    socketHandle = 0;
    isValid = false;
    hasRemoteAddr = false;
    memset(&remoteAddr, 0, sizeof(remoteAddr));
}

UdpSocket::~UdpSocket() {
    closeSocket();
}

bool UdpSocket::open() {
    SOCKET newSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (newSocket == INVALID_SOCKET) return false;
    socketHandle = (unsigned long long)newSocket;
    isValid = true;
    return true;
}

bool UdpSocket::bindToPort(int port) {
    if (!open()) return false;
    
    sockaddr_in localAddr;
    memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = INADDR_ANY;
    localAddr.sin_port = htons(port);
    
    SOCKET s = (SOCKET)socketHandle;
    if (bind(s, (sockaddr*)&localAddr, sizeof(localAddr)) == SOCKET_ERROR) {
        closeSocket();
        return false;
    }
    return true;
}

int UdpSocket::getLocalPort() {
    if (!isValid) return -1;
    sockaddr_in localAddr;
    int addrLen = sizeof(localAddr);
    if (getsockname((SOCKET)socketHandle, (sockaddr*)&localAddr, &addrLen) == SOCKET_ERROR) {
        return -1;
    }
    return ntohs(localAddr.sin_port);
}

void UdpSocket::setRemoteAddress(const std::string& ip, int port) {
    memset(&remoteAddr, 0, sizeof(remoteAddr));
    remoteAddr.sin_family = AF_INET;
    remoteAddr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &remoteAddr.sin_addr);
    hasRemoteAddr = true;
}

bool UdpSocket::setReceiveTimeout(int ms) {
    if (!isValid) return false;
    DWORD timeout = ms;
    return setsockopt((SOCKET)socketHandle, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) != SOCKET_ERROR;
}

bool UdpSocket::sendData(const char* data, int length) {
    if (!isValid || !hasRemoteAddr) return false;
    int result = sendto((SOCKET)socketHandle, data, length, 0, (sockaddr*)&remoteAddr, sizeof(remoteAddr));
    return result != SOCKET_ERROR;
}

int UdpSocket::receiveData(char* buffer, int bufferSize) {
    if (!isValid) return -1;
    sockaddr_in senderAddr;
    int senderLen = sizeof(senderAddr);
    int bytesRead = recvfrom((SOCKET)socketHandle, buffer, bufferSize, 0, (sockaddr*)&senderAddr, &senderLen);
    
    // Nếu chưa set địa chỉ đích (như Server chờ Client đầu tiên) thì ta lưu lại.
    if (bytesRead > 0 && !hasRemoteAddr) {
        remoteAddr = senderAddr;
        hasRemoteAddr = true;
    }
    return bytesRead;
}

bool UdpSocket::isConnected() { return isValid; }

void UdpSocket::closeSocket() {
    if (isValid) {
        closesocket((SOCKET)socketHandle);
        isValid = false;
    }
}
