#include "services/dir_navigate_service.h"

#include "protocol/reply_codes.h"
#include "protocol/reply_formatter.h"
#include "utils/ftp_path.h"
#include <filesystem>

namespace fs = std::filesystem;

// Xử lý lệnh PWD: trả về thư mục hiện tại của session
std::string DirNavigateService::handlePwdCommand(std::shared_ptr<Session> session) {
    if (session->authState != AuthState::LOGGED_IN) return "530 Not logged in.\r\n";
    return ReplyFormatter::format257PWD(session->currentDirectory);
}

// Xử lý lệnh CWD: chuyển thư mục làm việc sang thư mục chỉ định
// Kiểm tra thư mục đó có thật trên filesystem không trước khi cập nhật session
std::string DirNavigateService::handleCwdCommand(std::shared_ptr<Session> session, const std::string& path) {
    if (session->authState != AuthState::LOGGED_IN) return "530 Not logged in.\r\n";
    if (path.empty()) return "501 Syntax error in parameters.\r\n";

    // Xây dựng đường dẫn tuyệt đối và kiểm tra bảo mật
    fs::path realPath = FtpPath::resolve(session->currentDirectory, path);
    if (realPath.empty()) return "550 Access denied.\r\n";

    // Kiểm tra thư mục đích có thật tồn tại trên disk không
    if (!fs::exists(realPath) || !fs::is_directory(realPath)) {
        return "550 Directory not found.\r\n";
    }

    // Tính lại đường dẫn ảo FTP tương ứng với vị trí trong ServerRoot
    fs::path root = FtpPath::getRoot();
    fs::path relativePart = fs::relative(realPath, root);

    // Cập nhật thư mục hiện tại của session (dùng forward slash, bắt đầu bằng /)
    std::string newVirtualDir = "/" + relativePart.generic_string();
    if (newVirtualDir == "/.") newVirtualDir = "/"; // xử lý edge case root

    session->currentDirectory = newVirtualDir;
    return std::string(REPLY_250) + "\r\n";
}

// Xử lý lệnh CDUP: chuyển lên thư mục cha
// Nếu đang ở root "/" thì không làm gì, trả về 250
std::string DirNavigateService::handleCdupCommand(std::shared_ptr<Session> session) {
    if (session->authState != AuthState::LOGGED_IN) return "530 Not logged in.\r\n";

    // Đã ở root, không lên cao hơn được
    if (session->currentDirectory == "/") {
        return std::string(REPLY_250) + "\r\n";
    }

    // Lấy đường dẫn thư mục cha trên filesystem thật và kiểm tra
    fs::path realPath = FtpPath::resolve(session->currentDirectory, "");
    if (realPath.empty()) {
        session->currentDirectory = "/";
        return std::string(REPLY_250) + "\r\n";
    }

    fs::path parentReal = realPath.parent_path();
    fs::path root = FtpPath::getRoot();

    // Không cho vượt ra ngoài root
    if (!FtpPath::isSafe(parentReal)) {
        session->currentDirectory = "/";
        return std::string(REPLY_250) + "\r\n";
    }

    // Cập nhật lại đường dẫn ảo FTP
    if (fs::equivalent(parentReal, root)) {
        session->currentDirectory = "/";
    } else {
        fs::path relativePart = fs::relative(parentReal, root);
        session->currentDirectory = "/" + relativePart.generic_string();
    }

    return std::string(REPLY_250) + "\r\n";
}
