#ifndef SERVICES_DIR_LIST_SERVICE_H
#define SERVICES_DIR_LIST_SERVICE_H

#include "common/session.h"
#include <string>
#include <memory>

class DirListService {
public:
    static std::string handleListCommand(std::shared_ptr<Session> session, const std::string& path);
    static std::string handleNlstCommand(std::shared_ptr<Session> session, const std::string& path);
    static std::string handleStatCommand(std::shared_ptr<Session> session, const std::string& path);
};
#endif // SERVICES_DIR_LIST_SERVICE_H
