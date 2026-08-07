#include "services/system_service.h"

// Xử lý lệnh NOOP: không thực hiện thao tác nào, chỉ giữ kết nối sống
std::string SystemService::handleNoopCommand(std::shared_ptr<Session> session)
{
    return "200 NOOP ok.\r\n";
}

// Xử lý lệnh QUIT: trả về reply 221 để server.cpp gửi rồi ngắt kết nối
std::string SystemService::handleQuitCommand(std::shared_ptr<Session> session)
{
    return "221 Goodbye.\r\n";
}

// Xử lý lệnh không hợp lệ hoặc chưa được hỗ trợ
std::string SystemService::handleUnknownCommand(std::shared_ptr<Session> session)
{
    return "500 Syntax error, command unrecognized.\r\n";
}
