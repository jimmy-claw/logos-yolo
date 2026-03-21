cmake_minimum_required(VERSION 3.16)
project(screenshot_capture LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick)

add_executable(screenshot_capture screenshot_capture.cpp)
target_link_libraries(screenshot_capture PRIVATE
    Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick)
