#include "services/hash_service.h"
#include "utils/md5.h"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

static std::string getFullPath(std::shared_ptr<Session> session, const std::string& path) {
    std::string rootFolder = "ServerRoot";
    std::string fullPath = rootFolder + session->currentDirectory;
    if (fullPath.back() != '/') fullPath += "/";
    fullPath += path;
    return fullPath;
}

std::string HashService::handleHashCommand(std::shared_ptr<Session> session, const std::string& fileName) {
    if (session->authState != AuthState::LOGGED_IN) return "530 Not logged in.\r\n";
    if (fileName.empty()) return "501 Syntax error in parameters.\r\n";
    
    std::string fullPath = getFullPath(session, fileName);
    
    if (!fs::exists(fullPath) || !fs::is_regular_file(fullPath)) {
        return "550 File not found.\r\n";
    }
    
    try {
        std::ifstream file(fullPath, std::ios::binary);
        std::vector<char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        std::string hashValue = computeMD5(buffer);
        
        std::ostringstream oss;
        oss << "213 " << hashValue << "\r\n";
        return oss.str();
    } catch (...) {
        return "550 Failed to hash file.\r\n";
    }
}
