#include "services/file_manage_service.h"

#include "protocol/reply_codes.h"
#include "utils/ftp_path.h"
#include <filesystem>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>

namespace fs = std::filesystem;

// Xử lý lệnh SIZE: trả về kích thước file thật trên disk (byte)
std::string FileManageService::handleSizeCommand(std::shared_ptr<Session> session, const std::string& fileName) {
    if (session->authState != AuthState::LOGGED_IN) return "530 Not logged in.\r\n";
    if (fileName.empty()) return "501 Syntax error in parameters.\r\n";

    fs::path realPath = FtpPath::resolve(session->currentDirectory, fileName);
    if (realPath.empty()) return "550 Access denied.\r\n";

    try {
        if (!fs::exists(realPath) || !fs::is_regular_file(realPath)) {
            return "550 File not found.\r\n";
        }
        auto size = fs::file_size(realPath);
        return "213 " + std::to_string(size) + "\r\n";
    } catch (...) {
        return "550 Failed to get file size.\r\n";
    }
}

// Xử lý lệnh MDTM: trả về thời gian chỉnh sửa cuối của file thật (định dạng YYYYMMDDhhmmss)
std::string FileManageService::handleMdtmCommand(std::shared_ptr<Session> session, const std::string& fileName) {
    if (session->authState != AuthState::LOGGED_IN) return "530 Not logged in.\r\n";
    if (fileName.empty()) return "501 Syntax error in parameters.\r\n";

    fs::path realPath = FtpPath::resolve(session->currentDirectory, fileName);
    if (realPath.empty()) return "550 Access denied.\r\n";

    try {
        if (!fs::exists(realPath) || !fs::is_regular_file(realPath)) {
            return "550 File not found.\r\n";
        }
        // Chuyển thời gian file_time_type sang system_clock để có thể dùng gmtime
        auto ftime = fs::last_write_time(realPath);
        auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
        std::time_t tt = std::chrono::system_clock::to_time_t(sctp);
        std::tm* gmt = std::gmtime(&tt);

        std::ostringstream oss;
        oss << "213 " << std::put_time(gmt, "%Y%m%d%H%M%S") << "\r\n";
        return oss.str();
    } catch (...) {
        return "550 Failed to get file modification time.\r\n";
    }
}

// Xử lý lệnh DELE: xóa file thật trên disk bên trong ServerRoot
std::string FileManageService::handleDeleCommand(std::shared_ptr<Session> session, const std::string& fileName) {
    if (session->authState != AuthState::LOGGED_IN) return "530 Not logged in.\r\n";
    if (fileName.empty()) return "501 Syntax error in parameters.\r\n";

    fs::path realPath = FtpPath::resolve(session->currentDirectory, fileName);
    if (realPath.empty()) return "550 Access denied.\r\n";

    try {
        if (!fs::exists(realPath) || !fs::is_regular_file(realPath)) {
            return "550 File not found.\r\n";
        }
        fs::remove(realPath);
        return std::string(REPLY_250) + "\r\n";
    } catch (...) {
        return "550 Failed to delete file.\r\n";
    }
}

// Xử lý lệnh RNFR: ghi nhớ đường dẫn file cần đổi tên (bước 1 của RNFR/RNTO)
std::string FileManageService::handleRnfrCommand(std::shared_ptr<Session> session, const std::string& oldName) {
    if (session->authState != AuthState::LOGGED_IN) return "530 Not logged in.\r\n";
    if (oldName.empty()) return "501 Syntax error in parameters.\r\n";

    fs::path realPath = FtpPath::resolve(session->currentDirectory, oldName);
    if (realPath.empty()) return "550 Access denied.\r\n";

    if (!fs::exists(realPath)) {
        return "550 File or directory not found.\r\n";
    }

    // Lưu đường dẫn tuyệt đối thật vào session để RNTO dùng
    session->renameFromPath = realPath.string();
    return std::string(REPLY_350) + "\r\n";
}

// Xử lý lệnh RNTO: đổi tên file thật trên disk (bước 2 của RNFR/RNTO)
std::string FileManageService::handleRntoCommand(std::shared_ptr<Session> session, const std::string& newName) {
    if (session->authState != AuthState::LOGGED_IN) return "530 Not logged in.\r\n";
    if (newName.empty()) return "501 Syntax error in parameters.\r\n";
    if (session->renameFromPath.empty()) return "503 Bad sequence of commands (use RNFR first).\r\n";

    fs::path destPath = FtpPath::resolve(session->currentDirectory, newName);
    if (destPath.empty()) {
        session->renameFromPath = "";
        return "550 Access denied.\r\n";
    }

    try {
        fs::rename(session->renameFromPath, destPath);
        session->renameFromPath = "";
        return std::string(REPLY_250) + "\r\n";
    } catch (...) {
        session->renameFromPath = "";
        return "550 Failed to rename file or directory.\r\n";
    }
}
