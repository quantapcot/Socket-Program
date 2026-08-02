#ifndef PROTOCOL_REPLY_CODES_H
#define PROTOCOL_REPLY_CODES_H

// Constants for 1xx-5xx FTP reply codes
#define REPLY_125 "125 Data connection already open; transfer starting."
#define REPLY_150 "150 File status okay; about to open data connection."
#define REPLY_200 "200 Command OK."
#define REPLY_220 "220 Service ready for new user."
#define REPLY_221 "221 Service closing control connection. Logged out if appropriate."
#define REPLY_226 "226 Closing data connection. Requested file action successful."
#define REPLY_227 "227 Entering Passive Mode (%s,%d,%d)."
#define REPLY_230 "230 User logged in, proceed."
#define REPLY_250 "250 Requested file action okay, completed."
#define REPLY_257 "257 \"%s\" created."
#define REPLY_331 "331 User name okay, need password."
#define REPLY_350 "350 Requested file action pending further information."
#define REPLY_421 "421 Service not available, closing control connection."
#define REPLY_425 "425 Can't open data connection."
#define REPLY_426 "426 Connection closed; transfer aborted."
#define REPLY_450 "450 Requested file action not taken. File unavailable."
#define REPLY_500 "500 Syntax error, command unrecognized."
#define REPLY_501 "501 Syntax error in parameters or arguments."
#define REPLY_502 "502 Command not implemented."
#define REPLY_503 "503 Bad sequence of commands."
#define REPLY_530 "530 Not logged in."
#define REPLY_550 "550 Requested action not taken. File unavailable."
#endif // PROTOCOL_REPLY_CODES_H
