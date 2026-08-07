#include "services/help_service.h"

#include <algorithm>
#include <string>
#include <unordered_map>
#include <utility>

// Xử lý lệnh HELP [command]:
// - Không có argument: liệt kê tất cả lệnh FTP được hỗ trợ (multiline reply chuẩn RFC 959)
// - Có argument: trả về syntax + mô tả chi tiết của lệnh đó
std::string HelpService::handleHelpCommand(std::shared_ptr<Session> session, const std::string& arg)
{
    // Mỗi entry lưu 2 phần: syntax và description, KHÔNG nhúng \r\n bên trong
    // để tránh lỗi client đọc reply bị lệch (out-of-sync trên control connection)
    static const std::unordered_map<std::string, std::pair<std::string, std::string>> helpTable =
    {
        {"USER", {"Syntax: USER <username>",         "Specify username for login."}},
        {"PASS", {"Syntax: PASS <password>",         "Specify password for login."}},
        {"PWD",  {"Syntax: PWD",                     "Display current working directory."}},
        {"CWD",  {"Syntax: CWD <directory>",         "Change current working directory."}},
        {"CDUP", {"Syntax: CDUP",                    "Move to parent directory."}},
        {"MKD",  {"Syntax: MKD <directory>",         "Create a directory."}},
        {"RMD",  {"Syntax: RMD <directory>",         "Remove a directory."}},
        {"LIST", {"Syntax: LIST [directory]",        "List directory contents."}},
        {"NLST", {"Syntax: NLST [directory]",        "List file names only."}},
        {"STAT", {"Syntax: STAT [path]",             "Display status information."}},
        {"SIZE", {"Syntax: SIZE <file>",             "Show file size in bytes."}},
        {"MDTM", {"Syntax: MDTM <file>",             "Show last modified time."}},
        {"TYPE", {"Syntax: TYPE A|I",                "Set transfer type (ASCII or Binary)."}},
        {"MODE", {"Syntax: MODE S",                  "Set transfer mode (Stream)."}},
        {"PORT", {"Syntax: PORT h1,h2,h3,h4,p1,p2", "Use Active data transfer mode."}},
        {"PASV", {"Syntax: PASV",                    "Use Passive data transfer mode."}},
        {"RETR", {"Syntax: RETR <file>",             "Download a file from server."}},
        {"STOR", {"Syntax: STOR <file>",             "Upload a file to server."}},
        {"STOU", {"Syntax: STOU",                    "Upload a file with unique server filename."}},
        {"APPE", {"Syntax: APPE <file>",             "Append data to an existing file."}},
        {"DELE", {"Syntax: DELE <file>",             "Delete a file on server."}},
        {"RNFR", {"Syntax: RNFR <file>",             "Rename from (followed by RNTO)."}},
        {"RNTO", {"Syntax: RNTO <file>",             "Rename to (used after RNFR)."}},
        {"HASH", {"Syntax: HASH <file>",             "Calculate MD5 hash of a file."}},
        {"ABOR", {"Syntax: ABOR",                    "Abort current data transfer."}},
        {"QUIT", {"Syntax: QUIT",                    "Disconnect from server."}},
        {"NOOP", {"Syntax: NOOP",                    "No operation, keeps connection alive."}},
        {"HELP", {"Syntax: HELP [command]",          "Display help information."}}
    };

    // Không có argument: liệt kê toàn bộ lệnh được hỗ trợ
    if (arg.empty())
    {
        return
            "214-The following commands are recognized.\r\n"
            "USER PASS QUIT NOOP PWD CWD CDUP MKD RMD\r\n"
            "LIST NLST STAT SIZE MDTM TYPE MODE PORT PASV\r\n"
            "RETR STOR STOU APPE DELE RNFR RNTO HASH ABOR HELP\r\n"
            "214 Help OK.\r\n";
    }

    // Có argument: tìm và trả về thông tin của lệnh đó
    std::string key = arg;
    std::transform(key.begin(), key.end(), key.begin(), ::toupper);

    auto it = helpTable.find(key);
    if (it == helpTable.end())
        return "504 Unknown HELP topic.\r\n";

    // Trả về multiline reply chuẩn FTP RFC 959:
    //   "214-<syntax>\r\n"
    //   "<description>\r\n"
    //   "214 End.\r\n"
    return "214-" + it->second.first + "\r\n"
         + it->second.second + "\r\n"
         + "214 End.\r\n";
}
