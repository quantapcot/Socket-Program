#include "services/dir_navigate_service.h"

#include "protocol/reply_codes.h"
#include "protocol/reply_formatter.h"

std::string DirNavigateService::handlePwdCommand(std::shared_ptr<Session> session) {
    if (session->authState != AuthState::LOGGED_IN) return "530 Not logged in.\r\n";
    return ReplyFormatter::format257PWD(session->currentDirectory);
}

std::string DirNavigateService::handleCwdCommand(std::shared_ptr<Session> session, const std::string& path) {
    if (session->authState != AuthState::LOGGED_IN) return "530 Not logged in.\r\n";
    
    std::string newPath = path;
    if (path.empty()) {
        return "501 Syntax error in parameters.\r\n";
    }
    
    // Nếu đường dẫn không bắt đầu bằng / thì nối vào thư mục hiện tại
    if (path[0] != '/') {
        if (session->currentDirectory.back() == '/') {
            newPath = session->currentDirectory + path;
        } else {
            newPath = session->currentDirectory + "/" + path;
        }
    }
    
    session->currentDirectory = newPath;
    return std::string(REPLY_250) + "\r\n";
}

std::string DirNavigateService::handleCdupCommand(std::shared_ptr<Session> session) {
    if (session->authState != AuthState::LOGGED_IN) return "530 Not logged in.\r\n";
    
    if (session->currentDirectory == "/") {
        return std::string(REPLY_250) + "\r\n";
    }
    
    size_t lastSlash = session->currentDirectory.find_last_of('/');
    if (lastSlash == std::string::npos || lastSlash == 0) {
        session->currentDirectory = "/";
    } else {
        session->currentDirectory = session->currentDirectory.substr(0, lastSlash);
    }
    
    return std::string(REPLY_250) + "\r\n";
}
