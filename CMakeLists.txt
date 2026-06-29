cmake_minimum_required(VERSION 3.14)
project(gmacrod)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_SOURCES
    src/main.cpp
    src/button_thread/button.cpp
    src/button_thread/button.hpp
    src/button_thread/recorder.hpp
    src/button_thread/virtual_keyboard.hpp
    src/lcd_thread/lcd.cpp
    src/lcd_thread/lcd.hpp
    src/globals.hpp
    src/json.hpp
    src/profile/profile.hpp
    src/profile/profile.cpp
    src/ipc_thread/ipc.cpp
    src/ipc_thread/ipc.hpp
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    src
    /usr/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    g15render
    g15daemon_client
    g15
)