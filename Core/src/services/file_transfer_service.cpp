#include "services/file_transfer_service.h"

#include "protocol/reply_codes.h"
#include "protocol/rdt_sender.h"
#include "protocol/rdt_receiver.h"
#include "utils/ftp_path.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <chrono>

namespace fs = std::filesystem;

// Hàm nội bộ: setup data socket tuỳ theo chế độ ACTIVE hoặc PASSIVE
static UdpSocket* setupDataSocket(std::shared_ptr<Session> session) {
    if (session->dataMode == DataMode::ACTIVE) {
        UdpSocket* sock = new UdpSocket();
        sock->open();
        sock->setRemoteAddress(session->clientDataIp, session->clientDataPort);
        return sock;
    }
    else if (session->dataMode == DataMode::PASSIVE) {
        return session->passiveSocket;
    }
    return nullptr;
}

// Hàm nội bộ: dọn dẹp data socket sau khi truyền xong
static void cleanupDataSocket(std::shared_ptr<Session> session, UdpSocket* sock) {
    if (session->dataMode == DataMode::ACTIVE) {
        delete sock;
    } else {
        delete sock;
        session->passiveSocket = nullptr;
        session->dataMode = DataMode::NONE;
    }
}

// Xử lý lệnh RETR: gửi file thật từ ServerRoot về client qua data channel
std::string FileTransferService::handleRetrCommand(std::shared_ptr<Session> session, const std::string& fileName) {
    if (session->authState != AuthState::LOGGED_IN) return "530 Not logged in.\r\n";
    if (session->dataMode == DataMode::NONE) return "425 Use PORT or PASV first.\r\n";
    if (fileName.empty()) return "501 Syntax error in parameters.\r\n";

    // Giải quyết đường dẫn thật và kiểm tra bảo mật
    fs::path realPath = FtpPath::resolve(session->currentDirectory, fileName);
    if (realPath.empty()) return "550 Access denied.\r\n";

    if (!fs::exists(realPath) || !fs::is_regular_file(realPath)) {
        return "550 File not found.\r\n";
    }

    UdpSocket* dataSock = setupDataSocket(session);
    if (!dataSock) return "425 Can't open data connection.\r\n";

    // Gửi 150 trước để client biết sắp có dữ liệu đến
    session->controlConnection.sendLine(std::string(REPLY_150) + "\r\n");

    // Chế độ PASSIVE: chờ nhận byte "chào hỏi" từ client để biết địa chỉ đích
    if (session->dataMode == DataMode::PASSIVE) {
        char dummy[1024];
        dataSock->setReceiveTimeout(5000);
        dataSock->receiveData(dummy, sizeof(dummy));
    }

    // Đọc toàn bộ file thật từ disk vào buffer rồi gửi qua RDT
    std::ifstream file(realPath, std::ios::binary);
    std::vector<char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    RdtSender sender(dataSock);
    if (sender.sendBuffer(buffer)) {
        cleanupDataSocket(session, dataSock);
        return std::string(REPLY_226) + "\r\n";
    } else {
        cleanupDataSocket(session, dataSock);
        return "426 Connection closed; transfer aborted.\r\n";
    }
}

// Xử lý lệnh STOR: nhận file từ client và ghi xuống disk thật bên trong ServerRoot
std::string FileTransferService::handleStorCommand(std::shared_ptr<Session> session, const std::string& fileName) {
    if (session->authState != AuthState::LOGGED_IN) return "530 Not logged in.\r\n";
    if (session->dataMode == DataMode::NONE) return "425 Use PORT or PASV first.\r\n";
    if (fileName.empty()) return "501 Syntax error in parameters.\r\n";

    fs::path realPath = FtpPath::resolve(session->currentDirectory, fileName);
    if (realPath.empty()) return "550 Access denied.\r\n";

    // Đảm bảo thư mục chứa file tồn tại
    fs::create_directories(realPath.parent_path());

    UdpSocket* dataSock = setupDataSocket(session);
    if (!dataSock) return "425 Can't open data connection.\r\n";

    session->controlConnection.sendLine(std::string(REPLY_150) + "\r\n");

    // Chế độ PASSIVE: chờ byte chào hỏi từ client
    if (session->dataMode == DataMode::PASSIVE) {
        char dummy[10];
        dataSock->receiveData(dummy, sizeof(dummy));
    }

    RdtReceiver receiver(dataSock);
    std::vector<char> buffer;
    if (receiver.receiveBuffer(buffer)) {
        // Ghi dữ liệu nhận được xuống file thật trên disk
        std::ofstream file(realPath, std::ios::binary);
        file.write(buffer.data(), buffer.size());
        file.close();

        cleanupDataSocket(session, dataSock);
        return std::string(REPLY_226) + "\r\n";
    } else {
        cleanupDataSocket(session, dataSock);
        return "426 Connection closed; transfer aborted.\r\n";
    }
}

// Xử lý lệnh STOU: nhận file từ client, tự sinh tên file duy nhất trên server
std::string FileTransferService::handleStouCommand(std::shared_ptr<Session> session) {
    if (session->authState != AuthState::LOGGED_IN) return "530 Not logged in.\r\n";
    if (session->dataMode == DataMode::NONE) return "425 Use PORT or PASV first.\r\n";

    // Tạo tên file duy nhất dựa trên timestamp
    std::string fileName = "stou_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + ".tmp";
    fs::path realPath = FtpPath::resolve(session->currentDirectory, fileName);
    if (realPath.empty()) return "550 Access denied.\r\n";

    fs::create_directories(realPath.parent_path());

    UdpSocket* dataSock = setupDataSocket(session);
    if (!dataSock) return "425 Can't open data connection.\r\n";

    // Gửi 150 kèm tên file sẽ được tạo (theo chuẩn FTP)
    session->controlConnection.sendLine("150 FILE: " + fileName + "\r\n");

    RdtReceiver receiver(dataSock);
    std::vector<char> buffer;
    if (receiver.receiveBuffer(buffer)) {
        std::ofstream file(realPath, std::ios::binary);
        file.write(buffer.data(), buffer.size());
        file.close();

        cleanupDataSocket(session, dataSock);
        return "226 Transfer complete. File stored as: " + fileName + "\r\n";
    } else {
        cleanupDataSocket(session, dataSock);
        return "426 Connection closed; transfer aborted.\r\n";
    }
}

// Xử lý lệnh APPE: nhận file từ client và nối thêm vào cuối file thật trên disk
std::string FileTransferService::handleAppeCommand(std::shared_ptr<Session> session, const std::string& fileName) {
    if (session->authState != AuthState::LOGGED_IN) return "530 Not logged in.\r\n";
    if (session->dataMode == DataMode::NONE) return "425 Use PORT or PASV first.\r\n";
    if (fileName.empty()) return "501 Syntax error in parameters.\r\n";

    fs::path realPath = FtpPath::resolve(session->currentDirectory, fileName);
    if (realPath.empty()) return "550 Access denied.\r\n";

    // Đảm bảo thư mục chứa file tồn tại (APPE có thể tạo file mới)
    fs::create_directories(realPath.parent_path());

    UdpSocket* dataSock = setupDataSocket(session);
    if (!dataSock) return "425 Can't open data connection.\r\n";

    session->controlConnection.sendLine(std::string(REPLY_150) + "\r\n");

    if (session->dataMode == DataMode::PASSIVE) {
        char dummy[1024];
        dataSock->setReceiveTimeout(5000);
        dataSock->receiveData(dummy, sizeof(dummy));
    }

    RdtReceiver receiver(dataSock);
    std::vector<char> buffer;
    if (receiver.receiveBuffer(buffer)) {
        // Cờ std::ios::app: mở file để ghi nối tiếp vào cuối (không ghi đè)
        std::ofstream file(realPath, std::ios::binary | std::ios::app);
        file.write(buffer.data(), buffer.size());
        file.close();

        cleanupDataSocket(session, dataSock);
        return std::string(REPLY_226) + "\r\n";
    } else {
        cleanupDataSocket(session, dataSock);
        return "426 Connection closed; transfer aborted.\r\n";
    }
}

// Xử lý lệnh ABOR: hủy truyền dữ liệu đang diễn ra
// Trong mô hình đồng bộ (synchronous), không thể interrupt thread đang chạy.
// Trả về 226 theo chuẩn FTP.
std::string FileTransferService::handleAborCommand(std::shared_ptr<Session> session) {
    return "226 Abort successful.\r\n";
}
