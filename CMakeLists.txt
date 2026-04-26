cmake_minimum_required(VERSION 3.16)
project(SlotMachine VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(CORE_SOURCES
    ${SRC_DIR}/core/Symbol.cpp
    ${SRC_DIR}/core/Reel.cpp
    ${SRC_DIR}/core/PayTable.cpp
    ${SRC_DIR}/core/GameState.cpp
    ${SRC_DIR}/core/SlotMachine.cpp
    ${SRC_DIR}/core/SoundEngine.cpp
    ${SRC_DIR}/core/RarityConfig.cpp
    ${SRC_DIR}/ui/DoubleUpWidget.cpp
    ${SRC_DIR}/ui/WinTableWidget.cpp
)

set(UI_SOURCES
    ${SRC_DIR}/ui/ReelWidget.cpp
    ${SRC_DIR}/ui/LowerScreen.cpp
    ${SRC_DIR}/ui/UpperScreen.cpp
    ${SRC_DIR}/ui/MainWindow.cpp
)

set(CORE_HEADERS
    ${SRC_DIR}/core/Symbol.h
    ${SRC_DIR}/core/Reel.h
    ${SRC_DIR}/core/PayTable.h
    ${SRC_DIR}/core/GameState.h
    ${SRC_DIR}/core/SlotMachine.h
    ${SRC_DIR}/core/SoundEngine.h
    ${SRC_DIR}/core/RarityConfig.h
    ${SRC_DIR}/ui/DoubleUpWidget.h
    ${SRC_DIR}/ui/WinTableWidget.h
)

set(UI_HEADERS
    ${SRC_DIR}/ui/ReelWidget.h
    ${SRC_DIR}/ui/LowerScreen.h
    ${SRC_DIR}/ui/UpperScreen.h
    ${SRC_DIR}/ui/MainWindow.h
)

add_executable(SlotMachine WIN32
    ${SRC_DIR}/main.cpp
    ${CORE_SOURCES}
    ${UI_SOURCES}
    ${CORE_HEADERS}
    ${UI_HEADERS}
)

target_link_libraries(SlotMachine PRIVATE Qt6::Widgets Qt6::Multimedia)

target_include_directories(SlotMachine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# ── Copy rarity.cfg next to executable ──────────────────────────────────────
add_custom_command(TARGET SlotMachine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SRC_DIR}/rarity.cfg"
        "$<TARGET_FILE_DIR:SlotMachine>/rarity.cfg"
    COMMENT "Copying rarity.cfg"
)

# ── Windows application icon ──────────────────────────────────────────────────
if(WIN32)
    configure_file(${SRC_DIR}/app.rc.in ${CMAKE_BINARY_DIR}/app.rc)
    target_sources(SlotMachine PRIVATE ${CMAKE_BINARY_DIR}/app.rc)
endif()
