#include "services/hash_service.h"
#include "utils/md5.h"
#include "utils/ftp_path.h"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

// Xử lý lệnh HASH: tính MD5 hash của file thật trên ServerRoot và trả về client
std::string HashService::handleHashCommand(std::shared_ptr<Session> session, const std::string& fileName) {
    if (session->authState != AuthState::LOGGED_IN) return "530 Not logged in.\r\n";
    if (fileName.empty()) return "501 Syntax error in parameters.\r\n";

    // Giải quyết đường dẫn thật và kiểm tra bảo mật
    fs::path realPath = FtpPath::resolve(session->currentDirectory, fileName);
    if (realPath.empty()) return "550 Access denied.\r\n";

    if (!fs::exists(realPath) || !fs::is_regular_file(realPath)) {
        return "550 File not found.\r\n";
    }

    try {
        // Đọc toàn bộ file thật từ disk
        std::ifstream file(realPath, std::ios::binary);
        std::vector<char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        // Tính MD5 hash bằng Windows Cryptography API
        std::string hashValue = computeMD5(buffer);

        std::ostringstream oss;
        oss << "213 " << hashValue << "\r\n";
        return oss.str();
    } catch (...) {
        return "550 Failed to hash file.\r\n";
    }
}
