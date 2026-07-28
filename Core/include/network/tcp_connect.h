#ifndef TCP_CONNECT_H
#define TCP_CONNECT_H

#include <string>
using namespace std;

// Phai goi 1 lan duy nhat trong main(), truoc khi dung bat cu class nao ben duoi
bool initNetwork();

// Goi 1 lan cuoi cung, truoc khi chuong trinh ket thuc
void cleanupNetwork();

// Class dai dien cho 1 KET NOI TCP da thiet lap.
// Ca Client (sau khi connect) va Server (sau khi accept 1 client) deu dung chung class nay.
class TcpConnection
{
private:
    unsigned long long socketHandle;  // so hieu socket that su (Windows quan ly ben trong)
    bool isValid;                     // co dang la 1 ket noi hop le hay khong

public:
    // Constructor mac dinh: chua co ket noi gi ca
    TcpConnection();

    // Constructor nhan vao 1 socket da co san (Server se dung constructor nay,
    // vi no nhan duoc socket tu TcpServer.acceptClient())
    TcpConnection(unsigned long long existingSocket);

    // Destructor: tu dong dong ket noi khi object bi huy (het pham vi su dung)
    ~TcpConnection();

    // Client goi ham nay de chu dong ket noi toi server
    bool connectToServer(string ip, int port);

    // Gui 1 dong chu qua ket noi (tu dong them ky tu xuong dong)
    bool sendLine(string text);

    // Nhan 1 dong chu tu ket noi (doc toi khi gap ky tu xuong dong)
    string receiveLine();

    // Kiem tra ket noi con hop le khong
    bool isConnected();

    // Chu dong dong ket noi (khong bat buoc goi tay, destructor se tu goi neu quen)
    void closeConnection();
};

#endif