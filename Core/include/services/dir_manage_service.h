#ifndef SERVICES_DIR_MANAGE_SERVICE_H
#define SERVICES_DIR_MANAGE_SERVICE_H

#include "common/session.h"
#include <string>
#include <memory>

class DirManageService {
public:
    static std::string handleMkdCommand(std::shared_ptr<Session> session, const std::string& dirName);
    static std::string handleRmdCommand(std::shared_ptr<Session> session, const std::string& dirName);
};
#endif // SERVICES_DIR_MANAGE_SERVICE_H
