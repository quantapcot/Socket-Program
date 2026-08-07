#include "utils/ftp_path.h"

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

namespace FtpPath {

    // Forward declaration de isSafe co the duoc goi trong resolve()
    bool isSafe(const fs::path& path);

    // Lay duong dan tuyet doi den thu muc ServerRoot.
    // ServerRoot duoc dat ngay canh Server.exe (trong thu muc CWD cua process khi chay).
    std::filesystem::path getRoot() {
        fs::path root = fs::current_path() / "ServerRoot";
        if (!fs::exists(root)) {
            fs::create_directories(root);
        }
        return root;
    }

    // Chuyen duong dan ao FTP sang duong dan tuyet doi tren disk.
    // currentDir: thu muc hien tai cua session, dang "/sub" hoac "/"
    // ftpRelativePath: ten file/folder client gui len
    // Tra ve fs::path rong neu phat hien path traversal tan cong.
    std::filesystem::path resolve(const std::string& currentDir, const std::string& ftpRelativePath) {
        fs::path root = getRoot();

        // Xay dung duong dan day du: root + currentDir + relativePath
        fs::path combined;
        if (!ftpRelativePath.empty() && ftpRelativePath[0] == '/') {
            // Duong dan tuyet doi FTP (bat dau bang /), gan truc tiep vao root
            combined = root / ftpRelativePath.substr(1);
        } else {
            // Duong dan tuong doi, gan vao thu muc hien tai
            std::string cur = currentDir;
            if (!cur.empty() && cur[0] == '/') cur = cur.substr(1);
            combined = root / cur / ftpRelativePath;
        }

        // Chuan hoa duong dan de loai bo .., ., //
        fs::path canonical;
        try {
            // weakly_canonical khong yeu cau path phai ton tai (khac canonical)
            canonical = fs::weakly_canonical(combined);
        } catch (...) {
            return fs::path();
        }

        // Bao mat: kiem tra ket qua co nam trong root khong
        if (!isSafe(canonical)) {
            return fs::path();
        }

        return canonical;
    }

    // Kiem tra path co nam ben trong ServerRoot khong.
    // Ngan chan cac duong dan nhu ../../Windows thoat ra ngoai.
    bool isSafe(const std::filesystem::path& path) {
        fs::path root = fs::weakly_canonical(getRoot());
        auto rootStr = root.string();
        auto pathStr = path.string();
        // pathStr phai bat dau bang rootStr
        return pathStr.rfind(rootStr, 0) == 0;
    }

} // namespace FtpPath
