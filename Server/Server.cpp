#include <iostream>
#include "network/tcp_connect.h"
#include "network/tcp_listen.h"
using namespace std;

int main()
{
    // Buoc 1: Khoi tao thu vien mang (bat buoc phai goi truoc)
    if (!initNetwork())
    {
        cout << "Khong the khoi tao mang" << endl;
        return 1;
    }

    // Buoc 2: Tao server, lang nghe o port 2121
    // (dung port lon hon 1024 de tranh xung dot voi cac dich vu he thong)
    TcpServer server;
    int port = 2121;

    if (!server.startListening(port))
    {
        cout << "Khong the khoi dong server" << endl;
        cleanupNetwork();
        return 1;
    }

    cout << "Server dang lang nghe tai port " << port << "..." << endl;
    cout << "Dang cho client ket noi..." << endl;

    // Buoc 3: Cho 1 client ket noi toi (dung im o day cho toi khi co client)
    TcpConnection client = server.acceptClient();

    if (!client.isConnected())
    {
        cout << "Accept client that bai" << endl;
        cleanupNetwork();
        return 1;
    }

    cout << "Co client vua ket noi!" << endl;

    // Buoc 4: Gui 1 dong chao mung toi client
    client.sendLine("220 Service ready");
    cout << "Da gui: 220 Service ready" << endl;

    // Buoc 5: Nhan 1 dong tu client va in ra
    string receivedText = client.receiveLine();
    cout << "Nhan duoc tu client: " << receivedText << endl;

    // Buoc 6: Don dep truoc khi thoat
    client.closeConnection();
    server.stopListening();
    cleanupNetwork();

    cout << "Server ket thuc." << endl;

    // Giu cua so console khong tu tat ngay, de doc duoc ket qua
    system("pause");
    return 0;
}