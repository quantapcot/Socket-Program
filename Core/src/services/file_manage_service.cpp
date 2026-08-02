#include "services/file_manage_service.h"

#include "protocol/reply_codes.h"
#include <filesystem>
#include <iostream>
#include <sstream>
#include <iomanip>

namespace fs = std::filesystem;

static std::string getFullPath(std::shared_ptr<Session> session, const std::string& path) {
    std::string rootFolder = "ServerRoot";
    std::string fullPath = rootFolder + session->currentDirectory;
    if (fullPath.back() != '/') fullPath += "/";
    fullPath += path;
    return fullPath;
}

std::string FileManageService::handleSizeCommand(std::shared_ptr<Session> session, const std::string& fileName) {
    if (session->authState != AuthState::LOGGED_IN) return "530 Not logged in.\r\n";
    if (fileName.empty()) return "501 Syntax error in parameters.\r\n";
    
    std::string fullPath = getFullPath(session, fileName);
    
    try {
        if (fs::exists(fullPath) && fs::is_regular_file(fullPath)) {
            auto size = fs::file_size(fullPath);
            return "213 " + std::to_string(size) + "\r\n";
        } else {
            return "550 File not found.\r\n";
        }
    } catch (...) {
        return "550 Failed to get file size.\r\n";
    }
}

std::string FileManageService::handleMdtmCommand(std::shared_ptr<Session> session, const std::string& fileName) {
    if (session->authState != AuthState::LOGGED_IN) return "530 Not logged in.\r\n";
    if (fileName.empty()) return "501 Syntax error in parameters.\r\n";
    
    std::string fullPath = getFullPath(session, fileName);
    
    try {
        if (fs::exists(fullPath) && fs::is_regular_file(fullPath)) {
            // Lấy thời gian chỉnh sửa cuối (trả về YYYYMMDDhhmmss)
            auto ftime = fs::last_write_time(fullPath);
            auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
            std::time_t tt = std::chrono::system_clock::to_time_t(sctp);
            std::tm* gmt = std::gmtime(&tt);
            
            std::ostringstream oss;
            oss << "213 " << std::put_time(gmt, "%Y%m%d%H%M%S") << "\r\n";
            return oss.str();
        } else {
            return "550 File not found.\r\n";
        }
    } catch (...) {
        return "550 Failed to get file modification time.\r\n";
    }
}

std::string FileManageService::handleDeleCommand(std::shared_ptr<Session> session, const std::string& fileName) {
    if (session->authState != AuthState::LOGGED_IN) return "530 Not logged in.\r\n";
    if (fileName.empty()) return "501 Syntax error in parameters.\r\n";
    
    std::string fullPath = getFullPath(session, fileName);
    
    try {
        if (fs::exists(fullPath) && fs::is_regular_file(fullPath)) {
            fs::remove(fullPath);
            return std::string(REPLY_250) + "\r\n";
        } else {
            return "550 File not found.\r\n";
        }
    } catch (...) {
        return "550 Failed to delete file.\r\n";
    }
}

std::string FileManageService::handleRnfrCommand(std::shared_ptr<Session> session, const std::string& oldName) {
    if (session->authState != AuthState::LOGGED_IN) return "530 Not logged in.\r\n";
    if (oldName.empty()) return "501 Syntax error in parameters.\r\n";
    
    std::string fullPath = getFullPath(session, oldName);
    
    if (fs::exists(fullPath)) {
        session->renameFromPath = fullPath;
        return std::string(REPLY_350) + "\r\n";
    } else {
        return "550 File or directory not found.\r\n";
    }
}

std::string FileManageService::handleRntoCommand(std::shared_ptr<Session> session, const std::string& newName) {
    if (session->authState != AuthState::LOGGED_IN) return "530 Not logged in.\r\n";
    if (newName.empty()) return "501 Syntax error in parameters.\r\n";
    if (session->renameFromPath.empty()) return "503 Bad sequence of commands (use RNFR first).\r\n";
    
    std::string fullPath = getFullPath(session, newName);
    
    try {
        fs::rename(session->renameFromPath, fullPath);
        session->renameFromPath = "";
        return std::string(REPLY_250) + "\r\n";
    } catch (...) {
        session->renameFromPath = "";
        return "550 Failed to rename file or directory.\r\n";
    }
}
