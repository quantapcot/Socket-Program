#include "services/file_transfer_service.h"

#include "protocol/reply_codes.h"
#include "protocol/rdt_sender.h"
#include "protocol/rdt_receiver.h"
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

static UdpSocket* setupDataSocket(std::shared_ptr<Session> session) {
    if (session->dataMode == DataMode::ACTIVE) {
        UdpSocket* sock = new UdpSocket();
        sock->open();
        sock->setRemoteAddress(session->clientDataIp, session->clientDataPort);
        return sock;
    } else if (session->dataMode == DataMode::PASSIVE) {
        UdpSocket* sock = session->passiveSocket;
        if (sock == nullptr) return nullptr;
        
        char dummy[1024];
        sock->setReceiveTimeout(5000);
        sock->receiveData(dummy, sizeof(dummy)); 
        return sock;
    }
    return nullptr;
}

static std::string getFullPath(std::shared_ptr<Session> session, const std::string& path) {
    std::string rootFolder = "ServerRoot";
    std::string fullPath = rootFolder + session->currentDirectory;
    if (fullPath.back() != '/') fullPath += "/";
    fullPath += path;
    return fullPath;
}

std::string FileTransferService::handleRetrCommand(std::shared_ptr<Session> session, const std::string& fileName) {
    if (session->authState != AuthState::LOGGED_IN) return "530 Not logged in.\r\n";
    if (session->dataMode == DataMode::NONE) return "425 Use PORT or PASV first.\r\n";
    if (fileName.empty()) return "501 Syntax error in parameters.\r\n";
    
    std::string fullPath = getFullPath(session, fileName);
    if (!fs::exists(fullPath) || !fs::is_regular_file(fullPath)) {
        return "550 File not found.\r\n";
    }
    
    UdpSocket* dataSock = setupDataSocket(session);
    if (!dataSock) return "425 Can't open data connection.\r\n";

    session->controlConnection.sendLine(REPLY_150);
    
    // Đọc file
    std::ifstream file(fullPath, std::ios::binary);
    std::vector<char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    
    RdtSender sender(dataSock);
    if (sender.sendBuffer(buffer)) {
        if (session->dataMode == DataMode::ACTIVE) delete dataSock;
        else {
            delete dataSock;
            session->passiveSocket = nullptr;
            session->dataMode = DataMode::NONE;
        }
        return std::string(REPLY_226) + "\r\n";
    } else {
        if (session->dataMode == DataMode::ACTIVE) delete dataSock;
        return "426 Connection closed; transfer aborted.\r\n";
    }
}

std::string FileTransferService::handleStorCommand(std::shared_ptr<Session> session, const std::string& fileName) {
    if (session->authState != AuthState::LOGGED_IN) return "530 Not logged in.\r\n";
    if (session->dataMode == DataMode::NONE) return "425 Use PORT or PASV first.\r\n";
    if (fileName.empty()) return "501 Syntax error in parameters.\r\n";
    
    std::string fullPath = getFullPath(session, fileName);
    
    UdpSocket* dataSock = setupDataSocket(session);
    if (!dataSock) return "425 Can't open data connection.\r\n";

    session->controlConnection.sendLine(REPLY_150);
    
    RdtReceiver receiver(dataSock);
    std::vector<char> buffer;
    if (receiver.receiveBuffer(buffer)) {
        std::ofstream file(fullPath, std::ios::binary);
        file.write(buffer.data(), buffer.size());
        file.close();
        
        if (session->dataMode == DataMode::ACTIVE) delete dataSock;
        else {
            delete dataSock;
            session->passiveSocket = nullptr;
            session->dataMode = DataMode::NONE;
        }
        return std::string(REPLY_226) + "\r\n";
    } else {
        if (session->dataMode == DataMode::ACTIVE) delete dataSock;
        return "426 Connection closed; transfer aborted.\r\n";
    }
}

std::string FileTransferService::handleStouCommand(std::shared_ptr<Session> session) {
    if (session->authState != AuthState::LOGGED_IN) return "530 Not logged in.\r\n";
    if (session->dataMode == DataMode::NONE) return "425 Use PORT or PASV first.\r\n";
    
    std::string fileName = "stou_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + ".tmp";
    std::string fullPath = getFullPath(session, fileName);
    
    UdpSocket* dataSock = setupDataSocket(session);
    if (!dataSock) return "425 Can't open data connection.\r\n";

    // Gửi 150 FILE: fileName
    std::string reply150 = "150 FILE: " + fileName + "\r\n";
    session->controlConnection.sendLine(reply150);
    
    RdtReceiver receiver(dataSock);
    std::vector<char> buffer;
    if (receiver.receiveBuffer(buffer)) {
        std::ofstream file(fullPath, std::ios::binary);
        file.write(buffer.data(), buffer.size());
        file.close();
        
        if (session->dataMode == DataMode::ACTIVE) delete dataSock;
        else {
            delete dataSock;
            session->passiveSocket = nullptr;
            session->dataMode = DataMode::NONE;
        }
        // Gửi 226 hoặc 250 chứa tên file
        return "226 Transfer complete. File generated: " + fileName + "\r\n";
    } else {
        if (session->dataMode == DataMode::ACTIVE) delete dataSock;
        return "426 Connection closed; transfer aborted.\r\n";
    }
}

std::string FileTransferService::handleAppeCommand(std::shared_ptr<Session> session, const std::string& fileName) {
    if (session->authState != AuthState::LOGGED_IN) return "530 Not logged in.\r\n";
    if (session->dataMode == DataMode::NONE) return "425 Use PORT or PASV first.\r\n";
    if (fileName.empty()) return "501 Syntax error in parameters.\r\n";
    
    std::string fullPath = getFullPath(session, fileName);
    
    UdpSocket* dataSock = setupDataSocket(session);
    if (!dataSock) return "425 Can't open data connection.\r\n";

    session->controlConnection.sendLine(REPLY_150);
    
    RdtReceiver receiver(dataSock);
    std::vector<char> buffer;
    if (receiver.receiveBuffer(buffer)) {
        std::ofstream file(fullPath, std::ios::binary | std::ios::app);
        file.write(buffer.data(), buffer.size());
        file.close();
        
        if (session->dataMode == DataMode::ACTIVE) delete dataSock;
        else {
            delete dataSock;
            session->passiveSocket = nullptr;
            session->dataMode = DataMode::NONE;
        }
        return std::string(REPLY_226) + "\r\n";
    } else {
        if (session->dataMode == DataMode::ACTIVE) delete dataSock;
        return "426 Connection closed; transfer aborted.\r\n";
    }
}

std::string FileTransferService::handleAborCommand(std::shared_ptr<Session> session) {
    // ABOR đơn giản ngắt truyền. Trong mô hình synchronous, việc hủy ngang sẽ hơi khó khăn trừ khi có thread khác interrupt.
    // Tạm thời trả về 226
    return "226 Abort successful.\r\n";
}
