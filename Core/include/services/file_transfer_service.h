#ifndef SERVICES_FILE_TRANSFER_SERVICE_H
#define SERVICES_FILE_TRANSFER_SERVICE_H

#include "common/session.h"
#include <string>
#include <memory>

class FileTransferService {
public:
    static std::string handleRetrCommand(std::shared_ptr<Session> session, const std::string& fileName);
    static std::string handleStorCommand(std::shared_ptr<Session> session, const std::string& fileName);
    static std::string handleStouCommand(std::shared_ptr<Session> session);
    static std::string handleAppeCommand(std::shared_ptr<Session> session, const std::string& fileName);
    static std::string handleAborCommand(std::shared_ptr<Session> session);
};
#endif // SERVICES_FILE_TRANSFER_SERVICE_H
