#ifndef PROTOCOL_COMMAND_PARSER_H
#define PROTOCOL_COMMAND_PARSER_H

#include <string>
#include <vector>
#include "protocol/command_types.h"

struct Command {
    CommandType type;
    std::string argument;
};

class CommandParser {
public:
    // Chuyển đổi một dòng lệnh thô từ client thành cấu trúc Command
    static Command parse(const std::string& rawLine);
    
    // Tách chuỗi theo khoảng trắng
    static std::vector<std::string> splitSpace(const std::string& str);
};
#endif // PROTOCOL_COMMAND_PARSER_H
