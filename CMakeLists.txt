cmake_minimum_required(VERSION 3.16)
project(YoloPlugin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Qml)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# ── Logos SDK (optional, for LOGOS_CORE_AVAILABLE builds) ────────────────────
set(LOGOS_CPP_SDK_ROOT "$ENV{LOGOS_CPP_SDK_ROOT}" CACHE PATH "Path to logos-cpp-sdk")
set(LOGOS_LIBLOGOS_ROOT "$ENV{LOGOS_LIBLOGOS_ROOT}" CACHE PATH "Path to logos-liblogos")

if(NOT "${LOGOS_CPP_SDK_ROOT}" STREQUAL "" AND NOT "${LOGOS_LIBLOGOS_ROOT}" STREQUAL "")
    set(LOGOS_CORE_AVAILABLE ON)
    add_library(logos_cpp_sdk INTERFACE)
    target_include_directories(logos_cpp_sdk INTERFACE
        ${LOGOS_CPP_SDK_ROOT}/include
        ${LOGOS_CPP_SDK_ROOT}/include/cpp
        ${LOGOS_CPP_SDK_ROOT}/include/core
    )
    find_library(LOGOS_SDK_LIB logos_sdk HINTS ${LOGOS_CPP_SDK_ROOT}/lib REQUIRED)
    find_library(LOGOS_CORE_LIB logos_core HINTS ${LOGOS_LIBLOGOS_ROOT}/lib REQUIRED)
    target_link_libraries(logos_cpp_sdk INTERFACE ${LOGOS_SDK_LIB} ${LOGOS_CORE_LIB})
    add_library(logos_core INTERFACE)
    target_include_directories(logos_core INTERFACE ${LOGOS_LIBLOGOS_ROOT}/include)
endif()

# ── Headless module (via logos-module-builder) ────────────────────────────────
if(DEFINED ENV{LOGOS_MODULE_BUILDER_ROOT})
    include($ENV{LOGOS_MODULE_BUILDER_ROOT}/cmake/LogosModule.cmake)
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
endif()

# ── IComponent UI plugin (for logos-app-poc / Basecamp) ──────────────────────
option(BUILD_UI_PLUGIN "Build IComponent UI plugin for Basecamp" OFF)

if(BUILD_UI_PLUGIN)
    find_package(Qt6 REQUIRED COMPONENTS Widgets Quick QuickWidgets)

    find_package(component-interfaces QUIET)
    if(NOT component-interfaces_FOUND)
        add_library(component-interfaces INTERFACE)
        target_include_directories(component-interfaces INTERFACE
            ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
        )
    endif()

    add_library(yolo_ui SHARED
        src/yolo_ui_component.cpp
        src/yolo_plugin.cpp
        src/yolo_board.cpp
        qml/qml.qrc
    )

    target_include_directories(yolo_ui PRIVATE src ${CMAKE_SOURCE_DIR})

    target_compile_definitions(yolo_ui PRIVATE
        YOLO_UI_BUILD
        YOLO_UI_METADATA_FILE="${CMAKE_CURRENT_SOURCE_DIR}/ui_metadata.json"
    )

    target_link_libraries(yolo_ui PRIVATE
        Qt6::Core Qt6::Widgets Qt6::Quick Qt6::Qml Qt6::QuickWidgets
        component-interfaces
    )

    if(LOGOS_CORE_AVAILABLE)
        find_package(Qt6 REQUIRED COMPONENTS RemoteObjects)
        target_compile_definitions(yolo_ui PRIVATE LOGOS_CORE_AVAILABLE)
        target_link_libraries(yolo_ui PRIVATE
            Qt6::RemoteObjects logos_cpp_sdk logos_core ${LOGOS_SDK_LIB}
        )
    endif()

    set_target_properties(yolo_ui PROPERTIES PREFIX "" OUTPUT_NAME "yolo")
endif()
