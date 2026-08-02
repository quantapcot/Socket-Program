#ifndef PROTOCOL_RDT_SENDER_H
#define PROTOCOL_RDT_SENDER_H

#include "network/udp_io.h"
#include <vector>
#include <string>
#include <cstdint>

// Lớp RdtSender thực hiện gửi dữ liệu tin cậy qua UDP (Stop-and-Wait)
class RdtSender {
private:
    UdpSocket* socket;
    uint32_t nextSeqNum; // 0 hoặc 1
    int timeoutMs;
    int maxRetries;

    // Hàm gửi 1 chunk đơn lẻ (đã bao gồm header) và chờ ACK
    bool sendChunkWithWait(const char* payload, size_t length, bool isFin);

public:
    RdtSender(UdpSocket* udpSocket, int timeout = 500, int retries = 10);
    ~RdtSender();

    // Gửi toàn bộ buffer dữ liệu (sẽ tự chia nhỏ)
    bool sendBuffer(const std::vector<char>& data);
    
    // Đọc từ file và gửi đi (chia nhỏ theo block)
    bool sendFile(const std::string& filePath);
};
#endif // PROTOCOL_RDT_SENDER_H
