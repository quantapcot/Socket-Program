#ifndef SERVICES_DIR_NAVIGATE_SERVICE_H
#define SERVICES_DIR_NAVIGATE_SERVICE_H

#include "common/session.h"
#include <string>
#include <memory>

class DirNavigateService {
public:
    static std::string handlePwdCommand(std::shared_ptr<Session> session);
    static std::string handleCwdCommand(std::shared_ptr<Session> session, const std::string& path);
    static std::string handleCdupCommand(std::shared_ptr<Session> session);
};
#endif // SERVICES_DIR_NAVIGATE_SERVICE_H
