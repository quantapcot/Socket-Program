#include "services/dir_manage_service.h"

#include "protocol/reply_codes.h"
#include "protocol/reply_formatter.h"
#include "utils/ftp_path.h"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

// Xử lý lệnh MKD: tạo thư mục mới trên filesystem thật bên trong ServerRoot
std::string DirManageService::handleMkdCommand(std::shared_ptr<Session> session, const std::string& dirName) {
    if (session->authState != AuthState::LOGGED_IN) return "530 Not logged in.\r\n";
    if (dirName.empty()) return "501 Syntax error in parameters.\r\n";

    // Giải quyết đường dẫn tuyệt đối và kiểm tra bảo mật
    fs::path realPath = FtpPath::resolve(session->currentDirectory, dirName);
    if (realPath.empty()) return "550 Access denied.\r\n";

    try {
        if (fs::exists(realPath)) {
            return "550 Directory already exists.\r\n";
        }
        fs::create_directories(realPath);

        // Tạo đường dẫn ảo FTP để trả về trong reply 257
        std::string virtualPath = session->currentDirectory;
        if (!virtualPath.empty() && virtualPath.back() != '/') virtualPath += "/";
        virtualPath += dirName;

        return ReplyFormatter::format257MKD(virtualPath);
    } catch (const std::exception& e) {
        return "550 Failed to create directory.\r\n";
    }
}

// Xử lý lệnh RMD: xóa thư mục trên filesystem thật
// Chỉ xóa thư mục trống (theo chuẩn FTP)
std::string DirManageService::handleRmdCommand(std::shared_ptr<Session> session, const std::string& dirName) {
    if (session->authState != AuthState::LOGGED_IN) return "530 Not logged in.\r\n";
    if (dirName.empty()) return "501 Syntax error in parameters.\r\n";

    fs::path realPath = FtpPath::resolve(session->currentDirectory, dirName);
    if (realPath.empty()) return "550 Access denied.\r\n";

    try {
        if (!fs::exists(realPath) || !fs::is_directory(realPath)) {
            return "550 Directory not found.\r\n";
        }
        if (!fs::is_empty(realPath)) {
            return "550 Directory not empty.\r\n";
        }
        fs::remove(realPath);
        return std::string(REPLY_250) + "\r\n";
    } catch (const std::exception& e) {
        return "550 Failed to remove directory.\r\n";
    }
}
