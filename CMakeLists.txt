cmake_minimum_required(VERSION 3.16)
project(upowertray VERSION 1.1.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Export compile commands for clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets DBus)
find_package(Qt6 QUIET COMPONENTS LinguistTools)

# Source files
set(SOURCES
    src/main.cpp
    src/batterytray.cpp
    src/batterytray.h
    src/settingsdialog.cpp
    src/settingsdialog.h
    src/batteryicon.cpp
    src/batteryicon.h
    src/upowerhelper.cpp
    src/upowerhelper.h
)

# Translation files (optional - requires Qt6LinguistTools)
if(Qt6LinguistTools_FOUND)
    set(TS_FILES
        translations/upowertray_en.ts
        translations/upowertray_es.ts
    )
    qt6_add_translation(QM_FILES ${TS_FILES})
    message(STATUS "Building with translations support")
else()
    message(STATUS "Qt6LinguistTools not found - building without translations")
    set(QM_FILES "")
endif()

add_executable(upowertray
    ${SOURCES}
    ${QM_FILES}
)

target_link_libraries(upowertray PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::DBus
)

# Installation
install(TARGETS upowertray DESTINATION bin)
install(FILES upowertray.desktop DESTINATION share/applications)
install(FILES icons/upowertray.svg DESTINATION share/icons/hicolor/scalable/apps)
if(Qt6LinguistTools_FOUND)
    install(FILES ${QM_FILES} DESTINATION share/upowertray/translations)
endif()

# Testing (optional)
option(BUILD_TESTING "Build unit tests" OFF)
if(BUILD_TESTING)
    enable_testing()
    find_package(Qt6 REQUIRED COMPONENTS Test)

    add_executable(test_batteryicon
        tests/test_batteryicon.cpp
        src/batteryicon.cpp
        src/batteryicon.h
    )
    target_link_libraries(test_batteryicon PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Test
    )
    add_test(NAME BatteryIconTests COMMAND test_batteryicon)

    add_executable(test_upowerhelper
        tests/test_upowerhelper.cpp
        src/upowerhelper.cpp
        src/upowerhelper.h
    )
    target_link_libraries(test_upowerhelper PRIVATE
        Qt6::Core
        Qt6::DBus
        Qt6::Test
    )
    add_test(NAME UPowerHelperTests COMMAND test_upowerhelper)
endif()
