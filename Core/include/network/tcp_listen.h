#ifndef TCP_LISTEN_H
#define TCP_LISTEN_H

#include "network/tcp_connect.h"
#include <string>
using namespace std;

// Class dai dien cho socket LANG NGHE cua Server.
// Nhiem vu duy nhat: dung o 1 port co dinh, cho client ket noi toi,
// va "san sinh" ra cac object TcpConnection moi cho tung client.
class TcpServer
{
private:
    unsigned long long serverSocketHandle;
    bool isRunning;

public:
    TcpServer();
    ~TcpServer();

    // Tao socket, bind vao port, bat che do lang nghe
    bool startListening(int port);

    // Dung cho toi khi co 1 client ket noi toi, tra ve 1 TcpConnection moi de noi chuyen voi client do
    TcpConnection acceptClient();

    // Dong socket lang nghe (khi server muon dung hoat dong)
    void stopListening();
};

#endif