cmake_minimum_required(VERSION 3.5)

project(PopTranslate VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets LinguistTools)

find_package(KF5GlobalAccel)
find_package(KF5GuiAddons)
find_package(KF5Wayland)
find_package(KF5WindowSystem)
find_package(KF5WidgetsAddons)

include(FetchContent)
FetchContent_Declare(
    QOnlineTranslator
    GIT_REPOSITORY https://github.com/crow-translate/QOnlineTranslator.git
    GIT_TAG 1.6.4
)
FetchContent_MakeAvailable(QOnlineTranslator)

set(TS_FILES poptranslate_zh_CN.ts)

set(PROJECT_SOURCES
    src/main.cpp
    src/myapplication.cpp src/myapplication.h
    src/popupdialog.cpp src/popupdialog.h src/popupdialog.ui
    src/poptranslate_dbus.cpp src/poptranslate_dbus.h
    src/settingwindow.cpp src/settingwindow.h src/settingwindow.ui
    src/defaultsettings.h
    ${TS_FILES}
)

add_executable(poptranslate
    ${PROJECT_SOURCES}
)

qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

target_link_libraries(poptranslate PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    KF5::GlobalAccel
    KF5::GuiAddons
    KF5::WaylandClient
    KF5::WindowSystem
    KF5::WidgetsAddons
    QOnlineTranslator)

install(TARGETS poptranslate DESTINATION bin)
install(FILES data/io.github.glaumar.PopTranslate.desktop DESTINATION share/applications)
install(FILES data/io.github.glaumar.PopTranslate.svg DESTINATION share/icons/hicolor/scalable/apps)
install(FILES io.github.glaumar.PopTranslate.metainfo.xml DESTINATION share/metainfo)