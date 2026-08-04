#include "services/dir_manage_service.h"

#include "protocol/reply_codes.h"
#include "protocol/reply_formatter.h"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

std::string DirManageService::handleMkdCommand(std::shared_ptr<Session> session, const std::string& dirName) {
    if (session->authState != AuthState::LOGGED_IN) return "530 Not logged in.\r\n";
    if (dirName.empty()) return "501 Syntax error in parameters.\r\n";
    
    // Đơn giản hóa: tạo thư mục tĩnh trên Server/ 
    // Trong thực tế cần map với thư mục thật (như D:/FTP/...)
    std::string rootFolder = "ServerRoot";
    fs::create_directory(rootFolder);
    
    std::string fullPath = rootFolder + session->currentDirectory;
    if (fullPath.back() != '/') fullPath += "/";
    fullPath += dirName;
    
    try {
        if (fs::create_directories(fullPath)) {
            std::string virtualPath = session->currentDirectory;
            if (virtualPath.back() != '/') virtualPath += "/";
            virtualPath += dirName;
            return ReplyFormatter::format257MKD(virtualPath);
        } else {
            return "550 Directory already exists or cannot be created.\r\n";
        }
    } catch (const std::exception& e) {
        return "550 Failed to create directory.\r\n";
    }
}

std::string DirManageService::handleRmdCommand(std::shared_ptr<Session> session, const std::string& dirName) {
    if (session->authState != AuthState::LOGGED_IN) return "530 Not logged in.\r\n";
    if (dirName.empty()) return "501 Syntax error in parameters.\r\n";
    
    std::string rootFolder = "ServerRoot";
    std::string fullPath = rootFolder + session->currentDirectory;
    if (fullPath.back() != '/') fullPath += "/";
    fullPath += dirName;
    
    try {
        if (fs::exists(fullPath) && fs::is_directory(fullPath)) {
            if (fs::is_empty(fullPath)) {
                fs::remove(fullPath);
                return std::string(REPLY_250) + "\r\n";
            } else {
                return "550 Directory not empty.\r\n";
            }
        } else {
            return "550 Directory not found.\r\n";
        }
    } catch (const std::exception& e) {
        return "550 Failed to remove directory.\r\n";
    }
}
