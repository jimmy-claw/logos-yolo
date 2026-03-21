cmake_minimum_required(VERSION 3.16)
project(yolo VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Qt6 ───────────────────────────────────────────────────────────────────────
find_package(Qt6 REQUIRED COMPONENTS Core Qml)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# ── logos-pipe (required for YoloBoard) ──────────────────────────────────────
set(LOGOS_PIPE_ROOT "" CACHE PATH "Path to logos-pipe source tree")

# ── Logos SDK + liblogos (optional — only needed for Logos Core integration) ──
set(LOGOS_CPP_SDK_ROOT "" CACHE PATH "Path to logos-cpp-sdk install")
set(LOGOS_LIBLOGOS_ROOT "" CACHE PATH "Path to logos-liblogos install")

if(NOT "${LOGOS_CPP_SDK_ROOT}" STREQUAL "" AND NOT "${LOGOS_LIBLOGOS_ROOT}" STREQUAL "")
    set(LOGOS_CORE_AVAILABLE ON)

    add_library(logos_cpp_sdk INTERFACE)
    target_include_directories(logos_cpp_sdk INTERFACE
        "${LOGOS_LIBLOGOS_ROOT}/include"
        "${LOGOS_CPP_SDK_ROOT}/include"
        "${LOGOS_CPP_SDK_ROOT}/include/cpp"
        "${LOGOS_CPP_SDK_ROOT}/include/core"
    )

    add_library(logos_core SHARED IMPORTED)
    set_target_properties(logos_core PROPERTIES
        IMPORTED_LOCATION "${LOGOS_LIBLOGOS_ROOT}/lib/liblogos_core.so"
    )

    find_library(LOGOS_SDK_LIB logos_sdk PATHS "${LOGOS_CPP_SDK_ROOT}/lib" NO_DEFAULT_PATH REQUIRED)
endif()

# ── Headless logoscore module plugin ─────────────────────────────────────────
option(BUILD_MODULE "Build headless logoscore plugin (no Qt Quick/GUI)" OFF)
if(BUILD_MODULE)
    if(NOT LOGOS_CPP_SDK_ROOT OR NOT LOGOS_LIBLOGOS_ROOT)
        message(FATAL_ERROR "BUILD_MODULE requires -DLOGOS_CPP_SDK_ROOT=<path> -DLOGOS_LIBLOGOS_ROOT=<path>")
    endif()

    find_package(Qt6 REQUIRED COMPONENTS Core Qml RemoteObjects)

    qt_add_plugin(yolo_plugin CLASS_NAME YoloPlugin)
    set_target_properties(yolo_plugin PROPERTIES
        PREFIX ""
        OUTPUT_NAME "yolo_plugin"
    )

    target_sources(yolo_plugin PRIVATE
        src/yolo_plugin.cpp
        src/yolo_plugin.h
        src/yolo.cpp
    )

    target_include_directories(yolo_plugin PRIVATE
        src
        ${CMAKE_SOURCE_DIR}
        "${LOGOS_LIBLOGOS_ROOT}/include"
        "${LOGOS_CPP_SDK_ROOT}/include"
        "${LOGOS_CPP_SDK_ROOT}/include/cpp"
        "${LOGOS_CPP_SDK_ROOT}/include/core"
    )

    find_library(MODULE_LOGOS_CORE_LIB logos_core
        HINTS "${LOGOS_LIBLOGOS_ROOT}/lib" NO_DEFAULT_PATH)
    target_link_libraries(yolo_plugin PRIVATE
        Qt6::Core Qt6::Qml Qt6::RemoteObjects
        ${MODULE_LOGOS_CORE_LIB}
    )

    target_compile_definitions(yolo_plugin PRIVATE
        QT_PLUGIN
        LOGOS_CORE_AVAILABLE
        YOLO_WRAPPER
    )

    if(APPLE)
        set_target_properties(yolo_plugin PROPERTIES
            BUILD_RPATH   "@loader_path"
            INSTALL_RPATH "@loader_path"
        )
    elseif(UNIX)
        set_target_properties(yolo_plugin PROPERTIES
            BUILD_RPATH   "$ORIGIN"
            INSTALL_RPATH "$ORIGIN"
        )
    endif()
endif()

# ── UI Plugin (IComponent for logos-app) ─────────────────────────────────────
option(BUILD_UI_PLUGIN "Build the IComponent UI plugin for logos-app" OFF)

if(BUILD_UI_PLUGIN)
    find_package(Qt6 REQUIRED COMPONENTS Widgets Quick QuickWidgets)

    # component-interfaces (IComponent.h)
    find_package(component-interfaces QUIET)
    if(NOT component-interfaces_FOUND)
        add_library(component-interfaces INTERFACE)
        target_include_directories(component-interfaces INTERFACE
            "${CMAKE_CURRENT_SOURCE_DIR}/interfaces"
        )
    endif()

    set(UI_TARGET yolo_ui)

    add_library(${UI_TARGET} SHARED
        src/yolo_ui_component.cpp
        src/yolo.cpp
        qml/yolo_ui.qrc
    )

    target_include_directories(${UI_TARGET} PRIVATE
        src
        ${CMAKE_SOURCE_DIR}
    )

    target_compile_definitions(${UI_TARGET} PRIVATE
        YOLO_UI_BUILD
        YOLO_UI_METADATA_FILE="${CMAKE_CURRENT_SOURCE_DIR}/ui_metadata.json"
    )

    target_link_libraries(${UI_TARGET} PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Quick
        Qt6::Qml
        Qt6::QuickWidgets
        component-interfaces
    )

    if(LOGOS_CORE_AVAILABLE)
        find_package(Qt6 REQUIRED COMPONENTS RemoteObjects)
        target_link_libraries(${UI_TARGET} PRIVATE
            Qt6::RemoteObjects
            logos_cpp_sdk
            logos_core
            ${LOGOS_SDK_LIB}
        )
        target_compile_definitions(${UI_TARGET} PRIVATE LOGOS_CORE_AVAILABLE)
    endif()

    if(APPLE)
        set_target_properties(${UI_TARGET} PROPERTIES
            BUILD_RPATH   "@loader_path"
            INSTALL_RPATH "@loader_path"
        )
    elseif(UNIX)
        set_target_properties(${UI_TARGET} PROPERTIES
            BUILD_RPATH   "$ORIGIN"
            INSTALL_RPATH "$ORIGIN"
        )
    endif()

    install(TARGETS ${UI_TARGET}
        LIBRARY DESTINATION lib
    )
endif()

# ── Tests (no SDK required — uses stubs) ─────────────────────────────────────
option(BUILD_TESTS "Build YoloBoard unit tests" OFF)

if(BUILD_TESTS)
    enable_testing()
    find_package(Qt6 REQUIRED COMPONENTS Test)

    if("${LOGOS_PIPE_ROOT}" STREQUAL "")
        message(FATAL_ERROR "BUILD_TESTS requires -DLOGOS_PIPE_ROOT=<path to logos-pipe>")
    endif()

    # logos-pipe sources needed by YoloBoard (compiled directly — no separate lib)
    set(LOGOS_PIPE_SOURCES
        "${LOGOS_PIPE_ROOT}/src/federated_channel.cpp"
        "${LOGOS_PIPE_ROOT}/src/channel_sync.cpp"
        "${LOGOS_PIPE_ROOT}/src/channel_indexer.cpp"
        "${LOGOS_PIPE_ROOT}/src/content_store.cpp"
    )

    # ── test_yolo_board ──
    add_executable(test_yolo_board
        tests/test_yolo_board.cpp
        src/yolo_board.cpp
        ${LOGOS_PIPE_SOURCES}
    )

    target_include_directories(test_yolo_board PRIVATE
        src
        tests                              # logos_api_client.h stub
        "${LOGOS_PIPE_ROOT}/src"            # logos-pipe headers
    )

    target_link_libraries(test_yolo_board PRIVATE
        Qt6::Core Qt6::Test
    )

    add_test(NAME test_yolo_board COMMAND test_yolo_board)

    # ── test_yolo_events ──
    add_executable(test_yolo_events
        tests/test_yolo_events.cpp
        src/yolo_board.cpp
        ${LOGOS_PIPE_SOURCES}
    )

    target_include_directories(test_yolo_events PRIVATE
        src
        tests
        "${LOGOS_PIPE_ROOT}/src"
    )

    target_link_libraries(test_yolo_events PRIVATE
        Qt6::Core Qt6::Test
    )

    add_test(NAME test_yolo_events COMMAND test_yolo_events)

    # ── test_yolo_federation ──
    add_executable(test_yolo_federation
        tests/test_yolo_federation.cpp
        src/yolo_board.cpp
        ${LOGOS_PIPE_SOURCES}
    )

    target_include_directories(test_yolo_federation PRIVATE
        src
        tests
        "${LOGOS_PIPE_ROOT}/src"
    )

    target_link_libraries(test_yolo_federation PRIVATE
        Qt6::Core Qt6::Test
    )

    add_test(NAME test_yolo_federation COMMAND test_yolo_federation)

    # ── test_yolo_event_lifecycle ──
    add_executable(test_yolo_event_lifecycle
        tests/test_yolo_event_lifecycle.cpp
        src/yolo_board.cpp
        src/yolo.cpp
        ${LOGOS_PIPE_SOURCES}
    )

    target_include_directories(test_yolo_event_lifecycle PRIVATE
        src
        tests
        "${LOGOS_PIPE_ROOT}/src"
    )

    target_link_libraries(test_yolo_event_lifecycle PRIVATE
        Qt6::Core Qt6::Test
    )

    add_test(NAME test_yolo_event_lifecycle COMMAND test_yolo_event_lifecycle)
endif()
