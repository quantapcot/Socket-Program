#ifndef SERVICES_SYSTEM_SERVICE_H
#define SERVICES_SYSTEM_SERVICE_H

#include <string>
#include <memory>
#include "common/session.h"

class SystemService {
public:
    // Xử lý lệnh NOOP: không làm gì, chỉ giữ kết nối sống
    static std::string handleNoopCommand(std::shared_ptr<Session> session);

    // Trả về reply 221 Goodbye để server.cpp gửi trước khi ngắt kết nối
    static std::string handleQuitCommand(std::shared_ptr<Session> session);

    // Trả về reply 500 khi client gửi lệnh không hợp lệ hoặc chưa được hỗ trợ
    static std::string handleUnknownCommand(std::shared_ptr<Session> session);
};

#endif // SERVICES_SYSTEM_SERVICE_H
