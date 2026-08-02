#ifndef COMMON_SESSION_H
#define COMMON_SESSION_H

#include <string>
#include "network/tcp_connect.h"
#include "network/udp_io.h"

enum class AuthState {
    NOT_LOGGED_IN,
    NEED_PASSWORD,
    LOGGED_IN
};

enum class TransferType {
    ASCII,
    BINARY
};

enum class DataMode {
    NONE,
    ACTIVE,
    PASSIVE
};

class Session {
public:
    int sessionId;
    TcpConnection controlConnection;
    std::string clientIp;
    
    std::string username;
    AuthState authState;
    std::string currentDirectory;
    std::string renameFromPath;
    
    TransferType type;
    DataMode dataMode;
    
    // For Active mode
    std::string clientDataIp;
    int clientDataPort;
    
    // For Passive mode
    int serverDataPort;
    UdpSocket* passiveSocket;

    Session(int id, TcpConnection conn, std::string ip);
    ~Session();
};
#endif // COMMON_SESSION_H
