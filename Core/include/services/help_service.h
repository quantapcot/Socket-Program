#ifndef SERVICES_HELP_SERVICE_H
#define SERVICES_HELP_SERVICE_H

#include <string>
#include <memory>
#include "common/session.h"

class HelpService {
public:
    // Xử lý lệnh HELP [command]:
    // - Không có argument: trả về multiline reply liệt kê tất cả lệnh
    // - Có argument: trả về syntax + mô tả của lệnh đó
    static std::string handleHelpCommand(std::shared_ptr<Session> session, const std::string& arg);
};

#endif // SERVICES_HELP_SERVICE_H
