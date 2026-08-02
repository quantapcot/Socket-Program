#include "services/auth_service.h"

#include "protocol/reply_codes.h"

std::string AuthService::handleUserCommand(std::shared_ptr<Session> session, const std::string& username) {
    if (session->authState == AuthState::LOGGED_IN) {
        return "530 User already logged in.\r\n";
    }
    
    // Lưu username tạm thời
    session->username = username;
    session->authState = AuthState::NEED_PASSWORD;
    
    return std::string(REPLY_331) + "\r\n";
}

std::string AuthService::handlePassCommand(std::shared_ptr<Session> session, const std::string& password) {
    if (session->authState == AuthState::LOGGED_IN) {
        return "530 User already logged in.\r\n";
    }
    
    if (session->authState != AuthState::NEED_PASSWORD) {
        return "503 Login with USER first.\r\n";
    }
    
    // Ở đây ta đơn giản hóa việc chứng thực (cho phép mọi user đăng nhập thành công)
    session->authState = AuthState::LOGGED_IN;
    return std::string(REPLY_230) + "\r\n";
}
