# Tu dong quet toan bo file .cpp trong src/ (khong can khai bao tung file thu cong)
file(GLOB_RECURSE CORE_SOURCES "src/*.cpp")

# Tao 1 thu vien tinh ten "Core" tu cac file .cpp vua quet duoc
add_library(Core STATIC ${CORE_SOURCES})

# Cho biet noi chua cac file .h, de code trong Core va code o Server/Client
# co the #include "network/tcp_connect.h" ma khong loi
target_include_directories(Core PUBLIC include)

# Tat std::byte de tranh xung dot voi kieu byte cua Windows Socket
target_compile_definitions(Core PUBLIC _HAS_STD_BYTE=0)

# Link san thu vien mang cua Windows, de Server/Client dung Core khong can tu link lai
if(WIN32)
    target_link_libraries(Core PUBLIC ws2_32)
endif()