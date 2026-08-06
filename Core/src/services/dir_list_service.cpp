#include "services/dir_list_service.h"

#include "protocol/reply_codes.h"
#include "protocol/rdt_sender.h"
#include <filesystem>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

// Hàm phụ trợ để setup UdpSocket dùng cho Data Channel
static UdpSocket* setupDataSocket(std::shared_ptr<Session> session) {
    if (session->dataMode == DataMode::ACTIVE) {
        UdpSocket* sock = new UdpSocket();
        sock->open();
        sock->setRemoteAddress(session->clientDataIp, session->clientDataPort);
        return sock;
    } else if (session->dataMode == DataMode::PASSIVE) {
        UdpSocket* sock = session->passiveSocket;
        if (sock == nullptr) return nullptr;
        // Chờ nhận 1 byte hoặc gói tin SYN từ Client để biết địa chỉ đích (vì UDP không kết nối)
        char dummy[1024];
        sock->setReceiveTimeout(5000);
        sock->receiveData(dummy, sizeof(dummy)); 
        return sock;
    }
    return nullptr;
}

std::string DirListService::handleListCommand(std::shared_ptr<Session> session, const std::string& path) {
    if (session->authState != AuthState::LOGGED_IN) return "530 Not logged in.\r\n";
    if (session->dataMode == DataMode::NONE) return "425 Use PORT or PASV first.\r\n";
    
    std::string targetPath = path.empty() ? session->currentDirectory : path;
    std::string fullPath = "ServerRoot" + targetPath;
    
    std::ostringstream listData;
    try {
        if (fs::exists(fullPath) && fs::is_directory(fullPath)) {
            for (const auto& entry : fs::directory_iterator(fullPath)) {
                // Đơn giản hóa format LIST (ví dụ: drwxr-xr-x 1 owner group size date name)
                bool isDir = entry.is_directory();
                auto size = isDir ? 0 : entry.file_size();
                listData << (isDir ? "d" : "-") << "rwxr-xr-x 1 ftp ftp " 
                         << size << " Jan 1 00:00 " << entry.path().filename().string() << "\r\n";
            }
        }
    } catch (...) {
        return "550 Failed to list directory.\r\n";
    }
    
    UdpSocket* dataSock = setupDataSocket(session);
    if (!dataSock) return "425 Can't open data connection.\r\n";

    // Gửi 150 qua control channel kèm \r\n để Client thoát block
    session->controlConnection.sendLine(std::string(REPLY_150) + "\r\n");

    // Nếu là chế độ Passive, Server phải chờ nhận 1 byte "chào hỏi" từ Client 
    // để UdpSocket biết được IP và Port của Client trước khi gửi file
    if (session->dataMode == DataMode::PASSIVE) {
        char dummy[10];
        dataSock->receiveData(dummy, sizeof(dummy));
    }
    
    std::string dataStr = listData.str();
    std::vector<char> buffer(dataStr.begin(), dataStr.end());
    
    RdtSender sender(dataSock);
    if (sender.sendBuffer(buffer)) {
        if (session->dataMode == DataMode::ACTIVE) {
            delete dataSock; // dọn dẹp nếu là active (tự mở socket mới)
        } else {
            session->passiveSocket = nullptr;
            delete dataSock; // Passive dùng xong 1 lần cũng đóng
            session->dataMode = DataMode::NONE;
        }
        return std::string(REPLY_226) + "\r\n";
    } else {
        if (session->dataMode == DataMode::ACTIVE) delete dataSock;
        return "426 Connection closed; transfer aborted.\r\n";
    }
}

std::string DirListService::handleNlstCommand(std::shared_ptr<Session> session, const std::string& path) {
    if (session->authState != AuthState::LOGGED_IN) return "530 Not logged in.\r\n";
    if (session->dataMode == DataMode::NONE) return "425 Use PORT or PASV first.\r\n";

    std::string targetPath = path.empty() ? session->currentDirectory : path;
    std::string fullPath = "ServerRoot" + targetPath;

    std::ostringstream listData;
    try {
        if (fs::exists(fullPath) && fs::is_directory(fullPath)) {
            for (const auto& entry : fs::directory_iterator(fullPath)) {
                listData << entry.path().filename().string() << "\r\n";
            }
        }
    }
    catch (...) {
        return "550 Failed to list directory.\r\n";
    }

    UdpSocket* dataSock = setupDataSocket(session);
    if (!dataSock) return "425 Can't open data connection.\r\n";

    // Fix 1: Thêm \r\n vào đuôi REPLY_150 để Client không bị treo
    session->controlConnection.sendLine(std::string(REPLY_150) + "\r\n");

    // Fix 2: Hứng byte chào hỏi từ Client giống hệt các lệnh truyền file
    if (session->dataMode == DataMode::PASSIVE) {
        char dummy[1024];
        dataSock->setReceiveTimeout(5000);
        dataSock->receiveData(dummy, sizeof(dummy));
    }

    std::string dataStr = listData.str();
    std::vector<char> buffer(dataStr.begin(), dataStr.end());

    RdtSender sender(dataSock);
    if (sender.sendBuffer(buffer)) {
        if (session->dataMode == DataMode::ACTIVE) delete dataSock;
        else {
            delete dataSock;
            session->passiveSocket = nullptr;
            session->dataMode = DataMode::NONE;
        }
        return std::string(REPLY_226) + "\r\n";
    }
    else {
        if (session->dataMode == DataMode::ACTIVE) delete dataSock;
        return "426 Connection closed; transfer aborted.\r\n";
    }
}

std::string DirListService::handleStatCommand(std::shared_ptr<Session> session, const std::string& path) {
    if (session->authState != AuthState::LOGGED_IN) return "530 Not logged in.\r\n";
    // STAT trả về thông tin trạng thái server hoặc thư mục qua control channel, không dùng data channel
    return "211-Server status:\r\n  Hybrid FTP Server is running.\r\n211 End of status.\r\n";
}
