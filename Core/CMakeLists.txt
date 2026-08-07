# Khai bao tuong minh tung file .cpp can bien dich (tranh GLOB tu dong quet file rac)
set(CORE_SOURCES
    src/common/session.cpp
    src/network/tcp_connect.cpp
    src/network/tcp_listen.cpp
    src/network/udp_io.cpp
    src/protocol/command_parser.cpp
    src/protocol/rdt_checksum.cpp
    src/protocol/rdt_receiver.cpp
    src/protocol/rdt_sender.cpp
    src/protocol/reply_formatter.cpp
    src/services/auth_service.cpp
    src/services/dir_list_service.cpp
    src/services/dir_manage_service.cpp
    src/services/dir_navigate_service.cpp
    src/services/file_manage_service.cpp
    src/services/file_transfer_service.cpp
    src/services/hash_service.cpp
    src/services/help_service.cpp
    src/services/mode_service.cpp
    src/services/session_manager.cpp
    src/services/system_service.cpp
    src/utils/md5.cpp
)

# Tao 1 thu vien tinh ten "Core" tu cac file .cpp vua khai bao
add_library(Core STATIC ${CORE_SOURCES})

# Cho biet noi chua cac file .h, de code trong Core va code o Server/Client
# co the #include "network/tcp_connect.h" ma khong loi
target_include_directories(Core PUBLIC include)

# Tat std::byte de tranh xung dot voi kieu byte cua Windows Socket
target_compile_definitions(Core PUBLIC _HAS_STD_BYTE=0)

# Link san thu vien mang cua Windows, de Server/Client dung Core khong can tu link lai
if(WIN32)
    target_link_libraries(Core PUBLIC ws2_32 advapi32)
endif()

# Trigger CMake reconfigure