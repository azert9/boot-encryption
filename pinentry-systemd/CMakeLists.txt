cmake_minimum_required(VERSION 3.21)
project(pinentry-systemd)

set(CMAKE_CXX_STANDARD 17)

add_executable(pinentry-systemd src/main.cpp src/prompt/FileDescriptor.cpp src/prompt/PinPrompter.cpp)
target_link_libraries(pinentry-systemd gpg-error assuan)
