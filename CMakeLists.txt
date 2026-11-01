cmake_minimum_required(VERSION 3.24)
project(JadzumiPlayerWindows VERSION 0.3.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
if(MSVC)
    add_compile_options(/utf-8)
endif()
option(JADZUMI_BUILD_APP "Build the Windows desktop application" ${WIN32})
include(CTest)
find_package(Qt6 6.4 REQUIRED COMPONENTS Core Gui Sql)
add_library(jadzumi_playback STATIC
    src/playback/MpvApi.cpp src/playback/MpvApi.h
    src/playback/Player.cpp src/playback/Player.h
    src/library/HistoryStore.cpp src/library/HistoryStore.h
    src/settings/Preferences.cpp src/settings/Preferences.h)
target_include_directories(jadzumi_playback PUBLIC src PRIVATE third_party/mpv/include)
target_link_libraries(jadzumi_playback PUBLIC Qt6::Core Qt6::Gui Qt6::Sql)
if(WIN32)
    target_link_libraries(jadzumi_playback PUBLIC user32)
    target_compile_definitions(jadzumi_playback PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
endif()
if(MSVC)
    target_compile_options(jadzumi_playback PRIVATE /W4 /permissive- /utf-8)
else()
    target_compile_options(jadzumi_playback PRIVATE -Wall -Wextra -Wpedantic)
endif()
if(JADZUMI_BUILD_APP)
    if(NOT WIN32 OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
        message(FATAL_ERROR "The app requires Windows x64. Use JADZUMI_BUILD_APP=OFF for portable backend tests.")
    endif()
    find_package(Qt6 6.8.3 EXACT REQUIRED COMPONENTS Gui Qml Quick QuickControls2)
    enable_language(RC)
    qt_add_executable(jadzumi WIN32 src/app/main.cpp src/rendering/VideoWindow.cpp
        src/rendering/VideoWindow.h src/platform/windows/app.rc)
    target_link_libraries(jadzumi PRIVATE jadzumi_playback Qt6::Gui Qt6::Qml
        Qt6::Quick Qt6::QuickControls2 user32 dwmapi)
    target_include_directories(jadzumi PRIVATE assets)
    target_compile_definitions(jadzumi PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN _WIN32_WINNT=0x0A00 WINVER=0x0A00)
    set_source_files_properties(qml/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
    qt_policy(SET QTP0004 NEW)
    qt_add_qml_module(jadzumi URI Jadzumi VERSION 1.0 RESOURCE_PREFIX /qt/qml
        QML_FILES qml/Main.qml qml/Theme.qml
        qml/components/Icon.qml qml/components/JButton.qml qml/components/Card.qml
        qml/components/JCombo.qml qml/components/PlayPause.qml qml/components/SettingsPanel.qml
        qml/components/PlaybackPanel.qml qml/components/FileDetails.qml
        qml/screens/HomeView.qml qml/screens/QueueView.qml qml/player/PlayerHud.qml
        RESOURCES assets/jadzumi.ico assets/jadzumi.svg)
    install(TARGETS jadzumi RUNTIME DESTINATION .)
endif()
if(BUILD_TESTING)
    find_package(Qt6 6.4 REQUIRED COMPONENTS Test)
    add_executable(playback_tests tests/PlaybackTests.cpp)
    target_include_directories(playback_tests PRIVATE third_party/mpv/include)
    target_compile_definitions(playback_tests PRIVATE JADZUMI_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures")
    target_link_libraries(playback_tests PRIVATE jadzumi_playback Qt6::Test)
    add_test(NAME playback_contract COMMAND playback_tests)
    set_tests_properties(playback_contract PROPERTIES TIMEOUT 90)
endif()
