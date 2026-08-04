#include "protocol/reply_formatter.h"

#include <sstream>

std::string ReplyFormatter::format227(const std::string& ip, int port) {
    // Chuyển IP 127.0.0.1 thành 127,0,0,1
    std::string ipComma = ip;
    for (char& c : ipComma) {
        if (c == '.') c = ',';
    }
    
    // Tính p1, p2
    int p1 = port / 256;
    int p2 = port % 256;
    
    std::ostringstream oss;
    oss << "227 Entering Passive Mode (" << ipComma << "," << p1 << "," << p2 << ").\r\n";
    return oss.str();
}

std::string ReplyFormatter::format257(const std::string& path) {
    std::ostringstream oss;
    oss << "257 \"" << path << "\"\r\n";
    return oss.str();
}

std::string ReplyFormatter::format257PWD(const std::string& path) {
    std::ostringstream oss;
    oss << "257 \"" << path << "\" is current directory.\r\n";
    return oss.str();
}

std::string ReplyFormatter::format257MKD(const std::string& path) {
    std::ostringstream oss;
    oss << "257 \"" << path << "\" created.\r\n";
    return oss.str();
}