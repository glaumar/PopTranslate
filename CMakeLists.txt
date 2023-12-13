cmake_minimum_required(VERSION 3.25)

# clear warming
# cmake_policy(SET CMP0148 NEW)

project(PopTranslate
    VERSION 0.4.0
    DESCRIPTION "Translate the text selected by the mouse on plasma wayland"
    HOMEPAGE_URL https://github.com/glaumar/PopTranslate
    LANGUAGES CXX)

set(APPLICATION_NAME "PopTranslate")
set(APPLICATION_ID "io.github.glaumar.PopTranslate")
set(DESKTOP_FILE_NAME ${APPLICATION_ID}.desktop)
set(APPLICATION_ICON_BASE_NAME ${APPLICATION_ID})
set(METAINFO_FILE_NAME ${APPLICATION_ID}.metainfo.xml)
set(EXE_NAME poptranslate)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines")

set(CMAKE_GENERATOR "Unix Makefiles" CACHE INTERNAL "" FORCE)

find_package(Qt5 REQUIRED COMPONENTS LinguistTools Concurrent REQUIRED)
find_package(KF5GlobalAccel REQUIRED)
find_package(KF5GuiAddons REQUIRED)
find_package(KF5Wayland REQUIRED)
find_package(KF5WindowSystem REQUIRED)
find_package(KF5WidgetsAddons REQUIRED)
find_package(KF5XmlGui REQUIRED)
find_package(QCoro5 REQUIRED COMPONENTS Core)

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

# generate translation files
set(TS_FILES i18n/poptranslate_zh_CN.ts)
qt_add_translation(QM_FILES ${TS_FILES})
add_custom_target(translations ALL DEPENDS ${QM_FILES})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
configure_file(src/poptranslate.h.in poptranslate.h)

set(PROJECT_SOURCES
    src/main.cpp
    src/appmain.cpp
    src/popupdialog.cpp
    src/poptranslate_dbus.cpp
    src/settingwindow.cpp
    src/screengrabber.cpp
    src/imagecropper.cpp
    src/ocr.cpp
    src/tts.cpp
    src/langcode2name.cpp
    src/lang2iso639.cpp
    src/pageindicator.cpp
    src/poptranslatesettings.cpp
    src/abstracttranslator.h
    src/onlinetranslator.cpp
    src/translatormanager.cpp
    ${TS_FILES}
)

include_directories(src)

add_executable(${EXE_NAME} ${PROJECT_SOURCES})
add_dependencies(${EXE_NAME} translations)

target_include_directories(${EXE_NAME} PRIVATE 
                                        ${Tesseract_INCLUDE_DIRS} 
                                        ${Leptonica_INCLUDE_DIRS})

target_link_libraries(${EXE_NAME} PRIVATE
    Qt5::Concurrent
    KF5::GlobalAccel
    KF5::GuiAddons
    KF5::WaylandClient
    KF5::WindowSystem
    KF5::WidgetsAddons
    KF5::XmlGui
    QOnlineTranslator
    ${Tesseract_LIBRARIES}
    ${Leptonica_LIBRARIES}
    QCoro5::Core)

install(TARGETS ${EXE_NAME} DESTINATION bin)
install(FILES data/${DESKTOP_FILE_NAME} DESTINATION share/applications)
install(FILES data/${APPLICATION_ICON_BASE_NAME}.svg DESTINATION share/icons/hicolor/scalable/apps)
install(FILES ${METAINFO_FILE_NAME} DESTINATION share/metainfo)
install(FILES ${QM_FILES} DESTINATION share/PopTranslate/i18n)

# CPack
include(DEB/deb.cmake)
