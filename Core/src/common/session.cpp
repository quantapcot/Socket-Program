#include "common/session.h"

Session::Session(int id, TcpConnection conn, std::string ip) 
    : sessionId(id), controlConnection(conn), clientIp(ip) {
    authState = AuthState::NOT_LOGGED_IN;
    currentDirectory = "/";
    type = TransferType::ASCII;
    dataMode = DataMode::NONE;
    clientDataPort = 0;
    serverDataPort = 0;
    passiveSocket = nullptr;
}

Session::~Session() {
    if (passiveSocket != nullptr) {
        delete passiveSocket;
        passiveSocket = nullptr;
    }
}
