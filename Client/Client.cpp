#include <iostream>
#include "network/tcp_connect.h"
using namespace std;

int main()
{
    // Buoc 1: Khoi tao thu vien mang
    if (!initNetwork())
    {
        cout << "Khong the khoi tao mang" << endl;
        return 1;
    }

    // Buoc 2: Ket noi toi server
    // "127.0.0.1" nghia la ket noi toi chinh may nay (localhost) - dung khi test tren 1 may
    TcpConnection connection;
    string serverIp = "127.0.0.1";
    int serverPort = 2121;

    cout << "Dang ket noi toi server " << serverIp << ":" << serverPort << "..." << endl;

    if (!connection.connectToServer(serverIp, serverPort))
    {
        cout << "Ket noi that bai" << endl;
        cleanupNetwork();
        return 1;
    }

    cout << "Ket noi thanh cong!" << endl;

    // Buoc 3: Nhan dong chao mung tu server
    string welcomeMessage = connection.receiveLine();
    cout << "Server noi: " << welcomeMessage << endl;

    // Buoc 4: Gui 1 dong test toi server
    connection.sendLine("USER admin");
    cout << "Da gui: USER admin" << endl;

    // Buoc 5: Don dep
    connection.closeConnection();
    cleanupNetwork();

    cout << "Client ket thuc." << endl;

    system("pause");
    return 0;
}