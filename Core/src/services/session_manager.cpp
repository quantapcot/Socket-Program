#include "services/session_manager.h"
#include <iostream>

SessionManager::SessionManager() : nextSessionId(1) {}

SessionManager& SessionManager::getInstance() {
    static SessionManager instance;
    return instance;
}

std::shared_ptr<Session> SessionManager::createSession(TcpConnection conn, const std::string& ip) {
    std::lock_guard<std::mutex> lock(mtx);
    int id = nextSessionId++;
    auto session = std::make_shared<Session>(id, conn, ip);
    activeSessions[id] = session;
    return session;
}

void SessionManager::removeSession(int sessionId) {
    std::lock_guard<std::mutex> lock(mtx);
    activeSessions.erase(sessionId);
}

std::shared_ptr<Session> SessionManager::getSession(int sessionId) {
    std::lock_guard<std::mutex> lock(mtx);
    auto it = activeSessions.find(sessionId);
    if (it != activeSessions.end()) {
        return it->second;
    }
    return nullptr;
}

void SessionManager::printActiveSessions() {
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "--- Bảng Session Đang Hoạt Động ---" << std::endl;
    for (auto const& [id, session] : activeSessions) {
        std::cout << "Session ID: " << id << " | IP: " << session->clientIp 
                  << " | Trạng thái đăng nhập: " << (session->authState == AuthState::LOGGED_IN ? session->username : "Chưa")
                  << " | Thư mục: " << session->currentDirectory << std::endl;
    }
    std::cout << "-----------------------------------" << std::endl;
}
