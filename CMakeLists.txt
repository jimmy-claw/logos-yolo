cmake_minimum_required(VERSION 3.14)
project(YoloPlugin LANGUAGES CXX)
if(DEFINED ENV{LOGOS_MODULE_BUILDER_ROOT})
    include($ENV{LOGOS_MODULE_BUILDER_ROOT}/cmake/LogosModule.cmake)
else()
    message(FATAL_ERROR "Set LOGOS_MODULE_BUILDER_ROOT")
endif()
logos_module(
    NAME yolo
    SOURCES
        src/yolo_interface.h
        src/yolo_plugin.h
        src/yolo_plugin.cpp
        src/yolo_board.h
        src/yolo_board.cpp
        qml/qml.qrc
)
