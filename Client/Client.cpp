#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include "network/tcp_connect.h"
#include "network/udp_io.h"
#include "protocol/rdt_sender.h"
#include "protocol/rdt_receiver.h"

using namespace std;

int main()
{
    if (!initNetwork())
    {
        cout << "Khong the khoi tao mang" << endl;
        return 1;
    }

    TcpConnection connection;
    string serverIp = "127.0.0.1";
    int serverPort = 2121;

    cout << "Nhap IP Server (mac dinh 127.0.0.1): ";
    string ipInput;
    getline(cin, ipInput);
    if (!ipInput.empty()) serverIp = ipInput;

    cout << "Dang ket noi toi server " << serverIp << ":" << serverPort << "..." << endl;

    if (!connection.connectToServer(serverIp, serverPort))
    {
        cout << "Ket noi that bai" << endl;
        cleanupNetwork();
        return 1;
    }

    cout << "Ket noi thanh cong!" << endl;

    string welcomeMessage = connection.receiveLine();
    cout << welcomeMessage << endl;

    UdpSocket* dataSocket = nullptr;
    bool isPassive = false;

    while (true) {
        cout << "ftp> ";
        string cmd;
        getline(cin, cmd);
        if (cmd.empty()) continue;

        // Nếu lệnh là STOR thì ta cần kiểm tra xem file local có tồn tại không trước khi gửi lệnh lên Server
        string cmdPrefix = "";
        if (cmd.length() >= 4) {
            cmdPrefix = cmd.substr(0, 4);
            for (char& c : cmdPrefix) c = toupper(c);
        }
        
        string fileName = "";
        if (cmd.length() > 5) fileName = cmd.substr(5);

        if (cmdPrefix == "STOR" || cmdPrefix == "APPE") {
            ifstream f(fileName, ios::binary);
            if (!f.is_open()) {
                cout << "Local file not found: " << fileName << endl;
                continue;
            }
            f.close();
        }

        connection.sendLine(cmd + "\r\n");
        if (cmdPrefix == "QUIT") {
            cout << connection.receiveLine() << endl;
            break;
        }

        string reply = connection.receiveLine();
        cout << reply << endl;

        // Phân tích mã phản hồi
        if (reply.length() >= 3) {
            string codeStr = reply.substr(0, 3);
            int code = 0;
            try { code = stoi(codeStr); } catch (...) {}

            if (code == 227) { // PASV response
                size_t start = reply.find('(');
                size_t end = reply.find(')');
                if (start != string::npos && end != string::npos) {
                    string pasvData = reply.substr(start + 1, end - start - 1);
                    int h1, h2, h3, h4, p1, p2;
                    char comma;
                    istringstream iss(pasvData);
                    if (iss >> h1 >> comma >> h2 >> comma >> h3 >> comma >> h4 >> comma >> p1 >> comma >> p2) {
                        ostringstream ipStream;
                        ipStream << h1 << "." << h2 << "." << h3 << "." << h4;
                        string dataIp = ipStream.str();
                        int dataPort = (p1 * 256) + p2;
                        
                        if (dataSocket) delete dataSocket;
                        dataSocket = new UdpSocket();
                        dataSocket->open();
                        dataSocket->setRemoteAddress(dataIp, dataPort);
                        isPassive = true;
                    }
                }
            } else if (code == 150) { // Data transfer starting
                if (!dataSocket) {
                    cout << "Data socket not initialized. Use PASV first." << endl;
                    continue;
                }
                
                if (isPassive) {
                    // Gửi dummy byte để server biết địa chỉ Client
                    dataSocket->sendData("!", 1);
                }

                if (cmdPrefix == "LIST" || cmdPrefix == "NLST" || cmdPrefix == "RETR") {
                    RdtReceiver receiver(dataSocket);
                    vector<char> buffer;
                    if (receiver.receiveBuffer(buffer)) {
                        if (cmdPrefix == "RETR") {
                            ofstream f(fileName, ios::binary);
                            f.write(buffer.data(), buffer.size());
                            f.close();
                            cout << "Received file " << fileName << " (" << buffer.size() << " bytes)." << endl;
                        } else {
                            string listData(buffer.begin(), buffer.end());
                            cout << listData;
                        }
                    } else {
                        cout << "Data transfer failed." << endl;
                    }
                } else if (cmdPrefix == "STOR" || cmdPrefix == "APPE") {
                    ifstream f(fileName, ios::binary);
                    vector<char> buffer((istreambuf_iterator<char>(f)), istreambuf_iterator<char>());
                    f.close();
                    
                    RdtSender sender(dataSocket);
                    if (sender.sendBuffer(buffer)) {
                        cout << "Sent file " << fileName << " (" << buffer.size() << " bytes)." << endl;
                    } else {
                        cout << "Data transfer failed." << endl;
                    }
                } else if (cmdPrefix == "STOU") {
                    ifstream f(fileName, ios::binary);
                    vector<char> buffer((istreambuf_iterator<char>(f)), istreambuf_iterator<char>());
                    f.close();
                    
                    RdtSender sender(dataSocket);
                    if (sender.sendBuffer(buffer)) {
                        cout << "Sent file unique (" << buffer.size() << " bytes)." << endl;
                    } else {
                        cout << "Data transfer failed." << endl;
                    }
                }
                
                // Đọc 226 Transfer complete từ control channel
                string finalReply = connection.receiveLine();
                cout << finalReply << endl;
                
                delete dataSocket;
                dataSocket = nullptr;
                isPassive = false;
            }
        }
    }

    if (dataSocket) delete dataSocket;
    connection.closeConnection();
    cleanupNetwork();

    cout << "Client ket thuc." << endl;
    return 0;
}