#include "protocol/rdt_receiver.h"
#include "protocol/rdt_header.h"
#include "protocol/rdt_checksum.h"
#include <iostream>
#include <fstream>

#define MAX_PAYLOAD_SIZE 1024

RdtReceiver::RdtReceiver(UdpSocket* udpSocket) {
    this->socket = udpSocket;
    this->expectedSeqNum = 0;
}

RdtReceiver::~RdtReceiver() {
}

bool RdtReceiver::receiveBuffer(std::vector<char>& data) {
    char recvBuffer[sizeof(RdtHeader) + MAX_PAYLOAD_SIZE];
    bool isFin = false;
    
    // Đặt timeout không giới hạn hoặc lớn để chờ dữ liệu
    socket->setReceiveTimeout(5000); 
    
    while (!isFin) {
        int bytesRead = socket->receiveData(recvBuffer, sizeof(recvBuffer));
        
        if (bytesRead >= sizeof(RdtHeader)) {
            RdtHeader* header = reinterpret_cast<RdtHeader*>(recvBuffer);
            char* payload = recvBuffer + sizeof(RdtHeader);
            size_t payloadLen = header->payload_len;
            
            // Kiểm tra tính toàn vẹn
            if (RdtChecksum::verify(*header, payload, payloadLen)) {
                
                // Gửi ACK lại cho Sender
                RdtHeader ackHeader;
                ackHeader.seq_num = 0;
                ackHeader.ack_num = header->seq_num;
                ackHeader.payload_len = 0;
                ackHeader.flags = RDT_FLAG_ACK;
                ackHeader.checksum = 0;
                ackHeader.checksum = RdtChecksum::calculate(ackHeader, nullptr, 0);
                
                socket->sendData(reinterpret_cast<char*>(&ackHeader), sizeof(ackHeader));
                
                // Kiểm tra xem có đúng sequence mong đợi không (để tránh xử lý lại gói tin trùng)
                if (header->seq_num == expectedSeqNum) {
                    if (payloadLen > 0) {
                        data.insert(data.end(), payload, payload + payloadLen);
                    }
                    if (header->flags & RDT_FLAG_FIN) {
                        isFin = true;
                    }
                    expectedSeqNum = 1 - expectedSeqNum; // Chuyển state
                }
            } else {
                std::cout << "RdtReceiver: Checksum failed, dropping packet." << std::endl;
            }
        }
    }
    return true;
}

bool RdtReceiver::receiveFile(const std::string& filePath) {
    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        std::cout << "RdtReceiver: Cannot open file " << filePath << " for writing." << std::endl;
        return false;
    }
    
    char recvBuffer[sizeof(RdtHeader) + MAX_PAYLOAD_SIZE];
    bool isFin = false;
    
    // Timeout 10s cho việc truyền file
    socket->setReceiveTimeout(10000);
    
    while (!isFin) {
        int bytesRead = socket->receiveData(recvBuffer, sizeof(recvBuffer));
        
        if (bytesRead >= sizeof(RdtHeader)) {
            RdtHeader* header = reinterpret_cast<RdtHeader*>(recvBuffer);
            char* payload = recvBuffer + sizeof(RdtHeader);
            size_t payloadLen = header->payload_len;
            
            if (RdtChecksum::verify(*header, payload, payloadLen)) {
                
                RdtHeader ackHeader;
                ackHeader.seq_num = 0;
                ackHeader.ack_num = header->seq_num;
                ackHeader.payload_len = 0;
                ackHeader.flags = RDT_FLAG_ACK;
                ackHeader.checksum = 0;
                ackHeader.checksum = RdtChecksum::calculate(ackHeader, nullptr, 0);
                
                socket->sendData(reinterpret_cast<char*>(&ackHeader), sizeof(ackHeader));
                
                if (header->seq_num == expectedSeqNum) {
                    if (payloadLen > 0) {
                        file.write(payload, payloadLen);
                    }
                    if (header->flags & RDT_FLAG_FIN) {
                        isFin = true;
                    }
                    expectedSeqNum = 1 - expectedSeqNum;
                }
            }
        } else if (bytesRead < 0) {
            // Timeout hoặc lỗi socket
            std::cout << "RdtReceiver: Timeout or error receiving file data." << std::endl;
            file.close();
            return false;
        }
    }
    
    file.close();
    return true;
}
