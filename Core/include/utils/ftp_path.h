#ifndef UTILS_FTP_PATH_H
#define UTILS_FTP_PATH_H

#include <string>
#include <filesystem>

// FtpPath: module trung tâm quản lý đường dẫn FTP server
// - Tất cả service dùng hàm này để lấy đường dẫn tuyệt đối trên filesystem thật
// - Đảm bảo client không thể thoát ra ngoài ServerRoot (path traversal attack)
namespace FtpPath {

    // Trả về đường dẫn tuyệt đối đến ServerRoot trên filesystem thật.
    // ServerRoot được tạo ngay cạnh file Server.exe khi chạy.
    std::filesystem::path getRoot();

    // Chuyển đường dẫn ảo FTP (ví dụ "/subdir/file.txt") kết hợp với
    // currentDirectory của session thành đường dẫn tuyệt đối trên disk.
    // Trả về đường dẫn rỗng nếu phát hiện path traversal (../../ ...).
    std::filesystem::path resolve(const std::string& currentDir, const std::string& ftpRelativePath);

    // Kiểm tra xem đường dẫn có nằm trong ServerRoot không (bảo mật).
    bool isSafe(const std::filesystem::path& path);
}

#endif // UTILS_FTP_PATH_H
