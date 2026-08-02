#ifndef SERVICES_SESSION_MANAGER_H
#define SERVICES_SESSION_MANAGER_H

#include <map>
#include <mutex>
#include <memory>
#include "common/session.h"

// Singleton class để quản lý tất cả các session đang active trên Server
class SessionManager {
private:
    std::map<int, std::shared_ptr<Session>> activeSessions;
    std::mutex mtx;
    int nextSessionId;
    
    SessionManager();
public:
    static SessionManager& getInstance();
    
    // Tạo session mới và lưu vào danh sách
    std::shared_ptr<Session> createSession(TcpConnection conn, const std::string& ip);
    
    // Xóa session khi ngắt kết nối
    void removeSession(int sessionId);
    
    // Lấy thông tin session
    std::shared_ptr<Session> getSession(int sessionId);
    
    // In danh sách các session đang kết nối
    void printActiveSessions();
};
#endif // SERVICES_SESSION_MANAGER_H
