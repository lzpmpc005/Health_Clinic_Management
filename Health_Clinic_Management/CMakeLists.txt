cmake_minimum_required(VERSION 3.26)
project(Health_Clinic_Management_Web)

set(CMAKE_CXX_STANDARD 20)

include_directories(/usr/local/include)
include_directories(/opt/homebrew/Cellar/asio/1.28.1/include)
include_directories(/Users/charton/Git/Crow/include)
include_directories(/opt/homebrew/opt/sqlite/include)
include_directories(/opt/homebrew/Cellar/boost/1.83.0/include)


add_executable(Health_Clinic_Management_Web main.cpp
)

target_link_libraries(Health_Clinic_Management_Web pthread)
target_link_libraries(Health_Clinic_Management_Web /opt/homebrew/opt/sqlite/lib/libsqlite3.dylib)