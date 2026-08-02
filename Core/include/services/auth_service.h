#ifndef SERVICES_AUTH_SERVICE_H
#define SERVICES_AUTH_SERVICE_H

#include "common/session.h"
#include <string>
#include <memory>

class AuthService {
public:
    // Xử lý lệnh USER
    static std::string handleUserCommand(std::shared_ptr<Session> session, const std::string& username);
    
    // Xử lý lệnh PASS
    static std::string handlePassCommand(std::shared_ptr<Session> session, const std::string& password);
};
#endif // SERVICES_AUTH_SERVICE_H
