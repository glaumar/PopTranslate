cmake_minimum_required(VERSION 3.5)

project(PopTranslate VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools wayland)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)
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
    main.cpp
    myapplication.cpp myapplication.h
    popupdialog.cpp popupdialog.h popupdialog.ui
    poptranslate_dbus.cpp poptranslate_dbus.h
    settingwindow.cpp settingwindow.h settingwindow.ui
    defaultsettings.h
    ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(poptranslate
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    add_executable(poptranslate
        ${PROJECT_SOURCES}
    )

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(poptranslate PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    KF5::GlobalAccel
    KF5::GuiAddons
    KF5::WaylandClient
    KF5::WindowSystem
    KF5::WidgetsAddons
    QOnlineTranslator)

# install(TARGETS poptranslate
# BUNDLE DESTINATION .
# LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(poptranslate)
endif()
