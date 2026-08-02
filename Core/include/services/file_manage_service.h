#ifndef SERVICES_FILE_MANAGE_SERVICE_H
#define SERVICES_FILE_MANAGE_SERVICE_H

#include "common/session.h"
#include <string>
#include <memory>

class FileManageService {
public:
    static std::string handleSizeCommand(std::shared_ptr<Session> session, const std::string& fileName);
    static std::string handleMdtmCommand(std::shared_ptr<Session> session, const std::string& fileName);
    static std::string handleDeleCommand(std::shared_ptr<Session> session, const std::string& fileName);
    static std::string handleRnfrCommand(std::shared_ptr<Session> session, const std::string& oldName);
    static std::string handleRntoCommand(std::shared_ptr<Session> session, const std::string& newName);
};
#endif // SERVICES_FILE_MANAGE_SERVICE_H
