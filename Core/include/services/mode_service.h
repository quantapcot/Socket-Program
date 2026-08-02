#ifndef SERVICES_MODE_SERVICE_H
#define SERVICES_MODE_SERVICE_H

#include "common/session.h"
#include <string>
#include <memory>

class ModeService {
public:
    static std::string handleTypeCommand(std::shared_ptr<Session> session, const std::string& typeArgs);
    static std::string handleModeCommand(std::shared_ptr<Session> session, const std::string& modeArgs);
    static std::string handlePortCommand(std::shared_ptr<Session> session, const std::string& portArgs);
    static std::string handlePasvCommand(std::shared_ptr<Session> session);
};
#endif // SERVICES_MODE_SERVICE_H
