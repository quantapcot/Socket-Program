#include "services/mode_service.h"

#include "protocol/reply_codes.h"
#include "protocol/reply_formatter.h"
#include <iostream>
#include <sstream>

std::string ModeService::handleTypeCommand(std::shared_ptr<Session> session, const std::string& typeArgs) {
    if (session->authState != AuthState::LOGGED_IN) return "530 Not logged in.\r\n";
    
    if (typeArgs == "A" || typeArgs == "A N") {
        session->type = TransferType::ASCII;
        return std::string(REPLY_200) + "\r\n";
    } else if (typeArgs == "I") {
        session->type = TransferType::BINARY;
        return std::string(REPLY_200) + "\r\n";
    }
    
    return "504 Command not implemented for that parameter.\r\n";
}

std::string ModeService::handleModeCommand(std::shared_ptr<Session> session, const std::string& modeArgs) {
    if (session->authState != AuthState::LOGGED_IN) return "530 Not logged in.\r\n";
    
    // FTP truyền thống có Mode S (Stream), B (Block), C (Compressed)
    if (modeArgs == "S") {
        return std::string(REPLY_200) + "\r\n";
    }
    
    return "504 Command not implemented for that parameter.\r\n";
}

std::string ModeService::handlePortCommand(std::shared_ptr<Session> session, const std::string& portArgs) {
    if (session->authState != AuthState::LOGGED_IN) return "530 Not logged in.\r\n";
    
    // Phân tích PORT h1,h2,h3,h4,p1,p2
    int h1, h2, h3, h4, p1, p2;
    char comma;
    std::istringstream iss(portArgs);
    if (iss >> h1 >> comma >> h2 >> comma >> h3 >> comma >> h4 >> comma >> p1 >> comma >> p2) {
        std::ostringstream ipStream;
        ipStream << h1 << "." << h2 << "." << h3 << "." << h4;
        session->clientDataIp = ipStream.str();
        session->clientDataPort = (p1 * 256) + p2;
        session->dataMode = DataMode::ACTIVE;
        
        // Đóng socket passive nếu đang mở
        if (session->passiveSocket != nullptr) {
            delete session->passiveSocket;
            session->passiveSocket = nullptr;
        }
        
        return std::string(REPLY_200) + "\r\n";
    }
    
    return "501 Syntax error in parameters or arguments.\r\n";
}

std::string ModeService::handlePasvCommand(std::shared_ptr<Session> session) {
    if (session->authState != AuthState::LOGGED_IN) return "530 Not logged in.\r\n";
    
    if (session->passiveSocket != nullptr) {
        delete session->passiveSocket;
        session->passiveSocket = nullptr;
    }
    
    session->passiveSocket = new UdpSocket();
    // Khởi tạo passive socket (0 để HDH tự chọn port)
    if (!session->passiveSocket->bindToPort(0)) {
        delete session->passiveSocket;
        session->passiveSocket = nullptr;
        return "425 Can't open data connection.\r\n";
    }
    
    session->serverDataPort = session->passiveSocket->getLocalPort();
    session->dataMode = DataMode::PASSIVE;
    
    // Lấy IP của server (tạm giả sử localhost, thực tế cần lấy IP public/LAN của server)
    // Để đơn giản, ta trả về 127.0.0.1
    std::string ip = "127.0.0.1";
    
    return ReplyFormatter::format227(ip, session->serverDataPort);
}
