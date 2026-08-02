#ifndef SERVICES_HASH_SERVICE_H
#define SERVICES_HASH_SERVICE_H

#include "common/session.h"
#include <string>
#include <memory>

class HashService {
public:
    static std::string handleHashCommand(std::shared_ptr<Session> session, const std::string& fileName);
};
#endif // SERVICES_HASH_SERVICE_H
