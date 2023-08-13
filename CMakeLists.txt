cmake_minimum_required(VERSION 3.25)

project(PopTranslate VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_GENERATOR "Unix Makefiles" CACHE INTERNAL "" FORCE)

find_package(Qt5 REQUIRED COMPONENTS LinguistTools Concurrent REQUIRED)

# find_package(ECM REQUIRED)
# set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
# find_package(Wayland REQUIRED)
find_package(KF5GlobalAccel REQUIRED)
find_package(KF5GuiAddons REQUIRED)
find_package(KF5Wayland REQUIRED)
find_package(KF5WindowSystem REQUIRED)
find_package(KF5WidgetsAddons REQUIRED)
find_package(KF5XmlGui REQUIRED)

# Find Tesseract and Leptonica libraries using PkgConfig
find_package(PkgConfig REQUIRED)
pkg_check_modules(Tesseract REQUIRED tesseract)
pkg_check_modules(Leptonica REQUIRED lept)

# QOnlineTranslator
include(FetchContent)
FetchContent_Declare(
    QOnlineTranslator
    GIT_REPOSITORY https://github.com/crow-translate/QOnlineTranslator.git
    GIT_TAG 1.6.4
)
FetchContent_MakeAvailable(QOnlineTranslator)

# mdict-cpp
add_subdirectory(mdict-cpp)
include_directories(mdict-cpp)

# generate translation files
set(TS_FILES i18n/poptranslate_zh_CN.ts)
qt_add_translation(QM_FILES ${TS_FILES})
add_custom_target(translations ALL DEPENDS ${QM_FILES})

set(PROJECT_SOURCES
    src/main.cpp
    src/myapplication.cpp src/myapplication.h
    src/popupdialog.cpp src/popupdialog.h src/popupdialog.ui
    src/poptranslate_dbus.cpp src/poptranslate_dbus.h
    src/settingwindow.cpp src/settingwindow.h src/settingwindow.ui
    src/defaultsettings.h
    src/dictionaries.cpp src/dictionaries.h

    # src/wayland_primary_selection.cpp src/wayland_primary_selection.h
    src/screengrabber.h src/screengrabber.cpp
    src/imagecropper.h src/imagecropper.cpp
    src/ocr.h src/ocr.cpp
    src/langcode2name.h src/langcode2name.cpp
    ${TS_FILES}
)

# include_directories(${Qt5WaylandClient_PRIVATE_INCLUDE_DIRS})
add_executable(poptranslate
    ${PROJECT_SOURCES}
)
add_dependencies(poptranslate translations)

target_include_directories(poptranslate PRIVATE ${Tesseract_INCLUDE_DIRS} ${Leptonica_INCLUDE_DIRS})

target_link_libraries(poptranslate PRIVATE
    Qt5::Concurrent
    KF5::GlobalAccel
    KF5::GuiAddons
    KF5::WaylandClient
    KF5::WindowSystem
    KF5::WidgetsAddons
    KF5::XmlGui
    QOnlineTranslator

    # Wayland::Client
    # Qt5::WaylandClientPrivate
    ${Tesseract_LIBRARIES}
    ${Leptonica_LIBRARIES}
    mdict)

install(TARGETS poptranslate DESTINATION bin)
install(FILES data/io.github.glaumar.PopTranslate.desktop DESTINATION share/applications)
install(FILES data/io.github.glaumar.PopTranslate.svg DESTINATION share/icons/hicolor/scalable/apps)
install(FILES io.github.glaumar.PopTranslate.metainfo.xml DESTINATION share/metainfo)
install(FILES ${QM_FILES} DESTINATION share/PopTranslate/i18n)

# CPack
include(DEB/deb.cmake)
