#include "network/tcp_connect.h"
#include "network/tcp_listen.h"
#include "protocol/command_parser.h"
#include "services/auth_service.h"
#include "services/dir_list_service.h"
#include "services/dir_manage_service.h"
#include "services/dir_navigate_service.h"
#include "services/file_manage_service.h"
#include "services/file_transfer_service.h"
#include "services/hash_service.h"
#include "services/mode_service.h"
#include "services/session_manager.h"
#include <iostream>
#include <thread>
#include <vector>
#include <windows.h>


using namespace std;

void handleClient(TcpConnection client, string clientIp) {
  auto session = SessionManager::getInstance().createSession(client, clientIp);

  // Gửi lời chào 220
  client.sendLine("220 Hybrid FTP Server Ready.\r\n");
  SessionManager::getInstance().printActiveSessions();

  while (client.isConnected()) {
    string rawLine = client.receiveLine();
    if (rawLine.empty())
      break; // Client ngắt kết nối

    Command cmd = CommandParser::parse(rawLine);
    string reply;

    switch (cmd.type) {
    case CommandType::USER:
      reply = AuthService::handleUserCommand(session, cmd.argument);
      break;
    case CommandType::PASS:
      reply = AuthService::handlePassCommand(session, cmd.argument);
      break;
    case CommandType::PWD:
      reply = DirNavigateService::handlePwdCommand(session);
      break;
    case CommandType::CWD:
      reply = DirNavigateService::handleCwdCommand(session, cmd.argument);
      break;
    case CommandType::CDUP:
      reply = DirNavigateService::handleCdupCommand(session);
      break;
    case CommandType::MKD:
      reply = DirManageService::handleMkdCommand(session, cmd.argument);
      break;
    case CommandType::RMD:
      reply = DirManageService::handleRmdCommand(session, cmd.argument);
      break;
    case CommandType::LIST:
      reply = DirListService::handleListCommand(session, cmd.argument);
      break;
    case CommandType::NLST:
      reply = DirListService::handleNlstCommand(session, cmd.argument);
      break;
    case CommandType::STAT:
      reply = DirListService::handleStatCommand(session, cmd.argument);
      break;
    case CommandType::SIZE:
      reply = FileManageService::handleSizeCommand(session, cmd.argument);
      break;
    case CommandType::MDTM:
      reply = FileManageService::handleMdtmCommand(session, cmd.argument);
      break;
    case CommandType::DELE:
      reply = FileManageService::handleDeleCommand(session, cmd.argument);
      break;
    case CommandType::RNFR:
      reply = FileManageService::handleRnfrCommand(session, cmd.argument);
      break;
    case CommandType::RNTO:
      reply = FileManageService::handleRntoCommand(session, cmd.argument);
      break;
    case CommandType::TYPE:
      reply = ModeService::handleTypeCommand(session, cmd.argument);
      break;
    case CommandType::MODE:
      reply = ModeService::handleModeCommand(session, cmd.argument);
      break;
    case CommandType::PORT:
      reply = ModeService::handlePortCommand(session, cmd.argument);
      break;
    case CommandType::PASV:
      reply = ModeService::handlePasvCommand(session);
      break;
    case CommandType::RETR:
      reply = FileTransferService::handleRetrCommand(session, cmd.argument);
      break;
    case CommandType::STOR:
      reply = FileTransferService::handleStorCommand(session, cmd.argument);
      break;
    case CommandType::STOU:
      reply = FileTransferService::handleStouCommand(session);
      break;
    case CommandType::APPE:
      reply = FileTransferService::handleAppeCommand(session, cmd.argument);
      break;
    case CommandType::ABOR:
      reply = FileTransferService::handleAborCommand(session);
      break;
    case CommandType::HASH:
      reply = HashService::handleHashCommand(session, cmd.argument);
      break;
    case CommandType::QUIT:
      reply = "221 Goodbye.\r\n";
      client.sendLine(reply);
      goto end_loop;
    case CommandType::NOOP:
      reply = "200 NOOP ok.\r\n";
      break;
    case CommandType::HELP:
      reply = "214-The following commands are recognized.\r\n USER PASS QUIT "
              "NOOP PWD CWD CDUP MKD RMD LIST NLST STAT SIZE MDTM TYPE MODE "
              "PORT PASV RETR STOR STOU APPE DELE RNFR RNTO HASH ABOR "
              "HELP\r\n214 Help OK.\r\n";
      break;
    case CommandType::UNKNOWN:
    default:
      reply = "500 Syntax error, command unrecognized.\r\n";
      break;
    }

    if (!reply.empty()) {
      client.sendLine(reply);
    }
  }

end_loop:
  SessionManager::getInstance().removeSession(session->sessionId);
  client.closeConnection();
  SessionManager::getInstance().printActiveSessions();
}

int main() {
  SetConsoleOutputCP(CP_UTF8); // Set console to UTF-8
  if (!initNetwork()) {
    cout << "Khong the khoi tao mang" << endl;
    return 1;
  }

  TcpServer server;
  int port = 2121; // Default port

  if (!server.startListening(port)) {
    cout << "Khong the khoi dong server tren port " << port << endl;
    cleanupNetwork();
    return 1;
  }

  cout << "Hybrid FTP Server dang lang nghe tai port " << port << "..." << endl;

  // TODO: Maintain a list of thread objects if we want graceful shutdown
  while (true) {
    TcpConnection client = server.acceptClient();
    if (client.isConnected()) {
      string ip = client.getRemoteIP();
      cout << "Co client ket noi tu IP: " << ip << endl;
      std::thread t(handleClient, client, ip);
      t.detach(); // Để thread tự chạy độc lập
    } else {
      break;
    }
  }

  server.stopListening();
  cleanupNetwork();

  return 0;
}