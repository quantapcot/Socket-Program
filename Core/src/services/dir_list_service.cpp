#include "services/dir_list_service.h"

#include "protocol/reply_codes.h"
#include "protocol/rdt_sender.h"
#include "utils/ftp_path.h"
#include <filesystem>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>

namespace fs = std::filesystem;

// Hàm nội bộ: setup data socket theo chế độ ACTIVE hoặc PASSIVE
// Với PASSIVE: chờ nhận byte "chào hỏi" từ client để biết địa chỉ đích (vì UDP không tự kết nối)
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

// Hàm nội bộ: dọn dẹp data socket sau khi gửi xong
static void cleanupDataSocket(std::shared_ptr<Session> session, UdpSocket* sock) {
    if (session->dataMode == DataMode::ACTIVE) {
        delete sock;
    } else {
        session->passiveSocket = nullptr;
        delete sock;
        session->dataMode = DataMode::NONE;
    }
}

// Hàm nội bộ: định dạng thời gian file sang dạng "Mon DD HH:MM" (giống ls -l trên Unix)
static std::string formatFileTime(const fs::file_time_type& ftime) {
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
    std::time_t tt = std::chrono::system_clock::to_time_t(sctp);
    std::tm* gmt = std::gmtime(&tt);
    std::ostringstream oss;
    oss << std::put_time(gmt, "%b %d %H:%M");
    return oss.str();
}

// Xử lý lệnh LIST: liệt kê thư mục theo định dạng Unix ls -l từ filesystem thật
std::string DirListService::handleListCommand(std::shared_ptr<Session> session, const std::string& path) {
    if (session->authState != AuthState::LOGGED_IN) return "530 Not logged in.\r\n";
    if (session->dataMode == DataMode::NONE) return "425 Use PORT or PASV first.\r\n";

    // Xác định thư mục cần liệt kê
    std::string targetVirtualDir = path.empty() ? session->currentDirectory : path;
    fs::path realPath = FtpPath::resolve(session->currentDirectory, path.empty() ? "." : path);
    if (realPath.empty() || !fs::exists(realPath) || !fs::is_directory(realPath)) {
        return "550 Directory not found.\r\n";
    }

    // Xây dựng danh sách file theo định dạng Unix ls -l
    std::ostringstream listData;
    try {
        for (const auto& entry : fs::directory_iterator(realPath)) {
            bool isDir = entry.is_directory();
            auto size = isDir ? 0 : (uintmax_t)entry.file_size();
            std::string timeStr = formatFileTime(entry.last_write_time());
            std::string name = entry.path().filename().string();

            listData << (isDir ? "d" : "-") << "rwxr-xr-x 1 ftp ftp "
                     << std::setw(10) << size << " " << timeStr << " " << name << "\r\n";
        }
    } catch (...) {
        return "550 Failed to list directory.\r\n";
    }

    UdpSocket* dataSock = setupDataSocket(session);
    if (!dataSock) return "425 Can't open data connection.\r\n";

    session->controlConnection.sendLine(std::string(REPLY_150) + "\r\n");

    std::string dataStr = listData.str();
    std::vector<char> buffer(dataStr.begin(), dataStr.end());

    RdtSender sender(dataSock);
    if (sender.sendBuffer(buffer)) {
        cleanupDataSocket(session, dataSock);
        return std::string(REPLY_226) + "\r\n";
    } else {
        cleanupDataSocket(session, dataSock);
        return "426 Connection closed; transfer aborted.\r\n";
    }
}

// Xử lý lệnh NLST: liệt kê tên file/folder thuần túy (không có chi tiết) từ filesystem thật
std::string DirListService::handleNlstCommand(std::shared_ptr<Session> session, const std::string& path) {
    if (session->authState != AuthState::LOGGED_IN) return "530 Not logged in.\r\n";
    if (session->dataMode == DataMode::NONE) return "425 Use PORT or PASV first.\r\n";

    fs::path realPath = FtpPath::resolve(session->currentDirectory, path.empty() ? "." : path);
    if (realPath.empty() || !fs::exists(realPath) || !fs::is_directory(realPath)) {
        return "550 Directory not found.\r\n";
    }

    std::ostringstream listData;
    try {
        for (const auto& entry : fs::directory_iterator(realPath)) {
            listData << entry.path().filename().string() << "\r\n";
        }
    } catch (...) {
        return "550 Failed to list directory.\r\n";
    }

    UdpSocket* dataSock = setupDataSocket(session);
    if (!dataSock) return "425 Can't open data connection.\r\n";

    session->controlConnection.sendLine(std::string(REPLY_150) + "\r\n");

    std::string dataStr = listData.str();
    std::vector<char> buffer(dataStr.begin(), dataStr.end());

    RdtSender sender(dataSock);
    if (sender.sendBuffer(buffer)) {
        cleanupDataSocket(session, dataSock);
        return std::string(REPLY_226) + "\r\n";
    } else {
        cleanupDataSocket(session, dataSock);
        return "426 Connection closed; transfer aborted.\r\n";
    }
}

// Xử lý lệnh STAT: trả về thông tin trạng thái server và thư mục hiện tại qua control channel
// Không dùng data channel (theo RFC 959)
std::string DirListService::handleStatCommand(std::shared_ptr<Session> session, const std::string& path) {
    if (session->authState != AuthState::LOGGED_IN) return "530 Not logged in.\r\n";

    // Không có argument: trả về trạng thái server
    if (path.empty()) {
        return "211-Server status:\r\n"
               "211-Hybrid FTP Server is running.\r\n"
               "211-Connected user: " + session->username + "\r\n"
               "211-Current directory: " + session->currentDirectory + "\r\n"
               "211 End of status.\r\n";
    }

    // Có argument: trả về thông tin của file/thư mục chỉ định
    fs::path realPath = FtpPath::resolve(session->currentDirectory, path);
    if (realPath.empty() || !fs::exists(realPath)) {
        return "550 File or directory not found.\r\n";
    }

    std::ostringstream oss;
    oss << "213-Status of " << path << ":\r\n";
    if (fs::is_directory(realPath)) {
        oss << "213-Type: Directory\r\n";
        // Đếm số mục trong thư mục
        int count = 0;
        for (const auto& entry : fs::directory_iterator(realPath)) { count++; }
        oss << "213-Items: " << count << "\r\n";
    } else {
        oss << "213-Type: File\r\n";
        oss << "213-Size: " << fs::file_size(realPath) << " bytes\r\n";
    }
    oss << "213 End of status.\r\n";
    return oss.str();
}
