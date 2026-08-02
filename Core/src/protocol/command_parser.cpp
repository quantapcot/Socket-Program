#include "protocol/command_parser.h"

#include <sstream>
#include <algorithm>

std::vector<std::string> CommandParser::splitSpace(const std::string& str) {
    std::vector<std::string> result;
    std::istringstream iss(str);
    std::string token;
    while (iss >> token) {
        result.push_back(token);
    }
    return result;
}

Command CommandParser::parse(const std::string& rawLine) {
    Command cmd;
    cmd.type = CommandType::UNKNOWN;
    cmd.argument = "";
    
    if (rawLine.empty()) return cmd;
    
    // Xóa ký tự \r, \n ở cuối
    std::string line = rawLine;
    while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) {
        line.pop_back();
    }
    
    std::vector<std::string> parts = splitSpace(line);
    if (parts.empty()) return cmd;
    
    std::string cmdStr = parts[0];
    // Convert to uppercase for case-insensitive matching
    std::transform(cmdStr.begin(), cmdStr.end(), cmdStr.begin(), ::toupper);
    
    if (parts.size() > 1) {
        // argument is everything after the first space
        size_t spacePos = line.find(' ');
        if (spacePos != std::string::npos) {
            cmd.argument = line.substr(spacePos + 1);
            // remove leading spaces
            size_t start = cmd.argument.find_first_not_of(" ");
            if (start != std::string::npos) {
                cmd.argument = cmd.argument.substr(start);
            }
        }
    }
    
    if (cmdStr == "USER") cmd.type = CommandType::USER;
    else if (cmdStr == "PASS") cmd.type = CommandType::PASS;
    else if (cmdStr == "QUIT") cmd.type = CommandType::QUIT;
    else if (cmdStr == "NOOP") cmd.type = CommandType::NOOP;
    else if (cmdStr == "PWD") cmd.type = CommandType::PWD;
    else if (cmdStr == "CWD") cmd.type = CommandType::CWD;
    else if (cmdStr == "CDUP") cmd.type = CommandType::CDUP;
    else if (cmdStr == "MKD") cmd.type = CommandType::MKD;
    else if (cmdStr == "RMD") cmd.type = CommandType::RMD;
    else if (cmdStr == "LIST") cmd.type = CommandType::LIST;
    else if (cmdStr == "NLST") cmd.type = CommandType::NLST;
    else if (cmdStr == "STAT") cmd.type = CommandType::STAT;
    else if (cmdStr == "SIZE") cmd.type = CommandType::SIZE;
    else if (cmdStr == "MDTM") cmd.type = CommandType::MDTM;
    else if (cmdStr == "TYPE") cmd.type = CommandType::TYPE;
    else if (cmdStr == "MODE") cmd.type = CommandType::MODE;
    else if (cmdStr == "PORT") cmd.type = CommandType::PORT;
    else if (cmdStr == "PASV") cmd.type = CommandType::PASV;
    else if (cmdStr == "RETR") cmd.type = CommandType::RETR;
    else if (cmdStr == "STOR") cmd.type = CommandType::STOR;
    else if (cmdStr == "STOU") cmd.type = CommandType::STOU;
    else if (cmdStr == "APPE") cmd.type = CommandType::APPE;
    else if (cmdStr == "DELE") cmd.type = CommandType::DELE;
    else if (cmdStr == "RNFR") cmd.type = CommandType::RNFR;
    else if (cmdStr == "RNTO") cmd.type = CommandType::RNTO;
    else if (cmdStr == "HASH") cmd.type = CommandType::HASH;
    else if (cmdStr == "ABOR") cmd.type = CommandType::ABOR;
    else if (cmdStr == "HELP") cmd.type = CommandType::HELP;
    
    return cmd;
}
