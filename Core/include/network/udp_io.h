#ifndef NETWORK_UDP_IO_H
#define NETWORK_UDP_IO_H

#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

// Class đại diện cho UDP Socket dùng cho Data Channel
class UdpSocket {
private:
    unsigned long long socketHandle;
    bool isValid;
    sockaddr_in remoteAddr; // Lưu địa chỉ đích để send/recv
    bool hasRemoteAddr;

public:
    UdpSocket();
    ~UdpSocket();

    // Bind socket vào một port cụ thể (dành cho Server hoặc Passive Mode)
    bool bindToPort(int port);

    // Mở socket nhưng không bind (hệ điều hành tự chọn port, dành cho Client Active Mode)
    bool open();

    // Trả về port đang bind hiện tại
    int getLocalPort();

    // Đặt địa chỉ đích (để gửi/nhận dữ liệu)
    void setRemoteAddress(const std::string& ip, int port);
    
    // Đặt timeout cho recv (tính bằng milliseconds)
    bool setReceiveTimeout(int ms);

    // Gửi dữ liệu UDP
    bool sendData(const char* data, int length);

    // Nhận dữ liệu UDP (trả về số byte nhận được, hoặc -1 nếu lỗi/timeout)
    int receiveData(char* buffer, int bufferSize);

    bool isConnected();
    void closeSocket();
};
#endif // NETWORK_UDP_IO_H
