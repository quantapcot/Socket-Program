#ifndef PROTOCOL_RDT_RECEIVER_H
#define PROTOCOL_RDT_RECEIVER_H

#include "network/udp_io.h"
#include <vector>
#include <string>
#include <cstdint>

// Lớp RdtReceiver thực hiện nhận dữ liệu tin cậy qua UDP (Stop-and-Wait)
class RdtReceiver {
private:
    UdpSocket* socket;
    uint32_t expectedSeqNum;

public:
    RdtReceiver(UdpSocket* udpSocket);
    ~RdtReceiver();

    // Nhận dữ liệu vào buffer (dùng cho list directory, text nhỏ)
    bool receiveBuffer(std::vector<char>& data);
    
    // Nhận trực tiếp ra file
    bool receiveFile(const std::string& filePath);
};
#endif // PROTOCOL_RDT_RECEIVER_H
