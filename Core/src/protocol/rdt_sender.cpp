#include "protocol/rdt_sender.h"
#include "protocol/rdt_header.h"
#include "protocol/rdt_checksum.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <cstring>
#include <algorithm>

#define MAX_PAYLOAD_SIZE 1024

RdtSender::RdtSender(UdpSocket* udpSocket, int timeout, int retries) {
    this->socket = udpSocket;
    this->timeoutMs = timeout;
    this->maxRetries = retries;
    this->nextSeqNum = 0;
}

RdtSender::~RdtSender() {
}

bool RdtSender::sendChunkWithWait(const char* payload, size_t length, bool isFin) {
    char packet[sizeof(RdtHeader) + MAX_PAYLOAD_SIZE];
    RdtHeader* header = reinterpret_cast<RdtHeader*>(packet);
    
    header->seq_num = nextSeqNum;
    header->ack_num = 0;
    header->payload_len = static_cast<uint16_t>(length);
    header->flags = isFin ? RDT_FLAG_FIN : 0;
    header->checksum = 0;
    
    if (length > 0 && payload != nullptr) {
        memcpy(packet + sizeof(RdtHeader), payload, length);
    }
    
    header->checksum = RdtChecksum::calculate(*header, payload, length);
    
    int packetSize = sizeof(RdtHeader) + length;
    
    int retries = 0;
    char recvBuffer[sizeof(RdtHeader)];
    
    socket->setReceiveTimeout(timeoutMs);
    
    while (retries < maxRetries) {
        // Gửi gói tin
        if (!socket->sendData(packet, packetSize)) {
            return false;
        }
        
        // Chờ nhận ACK
        int bytesRead = socket->receiveData(recvBuffer, sizeof(recvBuffer));
        if (bytesRead >= sizeof(RdtHeader)) {
            RdtHeader* recvHeader = reinterpret_cast<RdtHeader*>(recvBuffer);
            
            // Kiểm tra checksum của ACK
            if (RdtChecksum::verify(*recvHeader, nullptr, 0)) {
                // Kiểm tra cờ ACK và số sequence
                if ((recvHeader->flags & RDT_FLAG_ACK) && (recvHeader->ack_num == nextSeqNum)) {
                    // Nhận đúng ACK, chuyển sang sequence tiếp theo (0 -> 1 -> 0)
                    nextSeqNum = 1 - nextSeqNum;
                    return true;
                }
            }
        }
        
        // Timeout hoặc sai ACK/checksum, lặp lại để retransmit
        retries++;
    }
    
    std::cout << "RdtSender: Max retries reached." << std::endl;
    return false;
}

bool RdtSender::sendBuffer(const std::vector<char>& data) {
    size_t offset = 0;
    size_t totalSize = data.size();
    
    if (totalSize == 0) {
        // Gửi gói rỗng với cờ FIN
        return sendChunkWithWait(nullptr, 0, true);
    }
    
    while (offset < totalSize) {
        size_t chunkLen = (std::min)(totalSize - offset, (size_t)MAX_PAYLOAD_SIZE);
        bool isFin = (offset + chunkLen == totalSize); // FIN ở gói cuối cùng
        
        if (!sendChunkWithWait(&data[offset], chunkLen, isFin)) {
            return false;
        }
        offset += chunkLen;
    }
    return true;
}

bool RdtSender::sendFile(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        std::cout << "RdtSender: Cannot open file " << filePath << std::endl;
        return false;
    }
    
    char buffer[MAX_PAYLOAD_SIZE];
    bool isFin = false;
    
    // Khởi tạo kích thước file để xác định packet cuối
    file.seekg(0, std::ios::end);
    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    
    if (fileSize == 0) {
        file.close();
        return sendChunkWithWait(nullptr, 0, true);
    }
    
    std::streamsize bytesSentTotal = 0;
    
    while (!isFin) {
        file.read(buffer, MAX_PAYLOAD_SIZE);
        std::streamsize bytesRead = file.gcount();
        
        bytesSentTotal += bytesRead;
        isFin = (bytesSentTotal >= fileSize);
        
        if (!sendChunkWithWait(buffer, static_cast<size_t>(bytesRead), isFin)) {
            file.close();
            return false;
        }
    }
    
    file.close();
    return true;
}
