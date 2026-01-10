cmake_minimum_required(VERSION 3.16)
project(upowertray VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets DBus)

add_executable(upowertray
    src/main.cpp
    src/batterytray.cpp
    src/batterytray.h
    src/settingsdialog.cpp
    src/settingsdialog.h
)

target_link_libraries(upowertray PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::DBus
)

install(TARGETS upowertray DESTINATION bin)
install(FILES upowertray.desktop DESTINATION share/applications)
install(FILES icons/upowertray.svg DESTINATION share/icons/hicolor/scalable/apps)
