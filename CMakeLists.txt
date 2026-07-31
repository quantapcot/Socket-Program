cmake_minimum_required(VERSION 3.20)
project(HybridFTP)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Nap lan luot cau hinh cua tung thu muc con
add_subdirectory(Core)
add_subdirectory(Server)
add_subdirectory(Client)