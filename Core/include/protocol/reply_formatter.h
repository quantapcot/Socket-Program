#ifndef PROTOCOL_REPLY_FORMATTER_H
#define PROTOCOL_REPLY_FORMATTER_H

#include <string>

class ReplyFormatter {
public:
    // Format 227 Passive Mode (ip, port)
    static std::string format227(const std::string& ip, int port);
    
    // Format 257 cho PWD / MKD
    static std::string format257(const std::string& path);

    static std::string format257PWD(const std::string& path);
    static std::string format257MKD(const std::string& path);
};
#endif // PROTOCOL_REPLY_FORMATTER_H
