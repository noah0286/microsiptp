cmake_minimum_required(VERSION 3.16)

project(microsiptp VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql Multimedia)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql Multimedia)

set(PROJECT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/base/global.h
    ${CMAKE_CURRENT_SOURCE_DIR}/base/global.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/telephonemainwindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/telephonemainwindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/telephonemainwindow.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/aboutdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/aboutdialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/aboutdialog.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/calldialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/calldialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/calldialog.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/guidedialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/guidedialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/guidedialog.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/preferencesmainwindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/preferencesmainwindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/preferencesmainwindow.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/transfercalldialog.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/transfercalldialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/transfercalldialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/kdialog.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/managermainwindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/managermainwindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ua/mycall.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ua/mycall.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ua/ua.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ua/ua.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ua/uamanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ua/uamanager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ua/telephone.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ua/telephone.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/res/resources.qrc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(microsiptp
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET microsiptp APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(microsiptp SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(microsiptp
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_include_directories(microsiptp
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/base
    ${CMAKE_CURRENT_SOURCE_DIR}/ua
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/include
)

target_link_libraries(microsiptp
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Multimedia
)

target_link_libraries(microsiptp
    PRIVATE
    debug ${CMAKE_CURRENT_SOURCE_DIR}/deps/lib/libpjproject-x86_64-x64-vc14-Debug-Dynamic.lib
    optimized ${CMAKE_CURRENT_SOURCE_DIR}/deps/lib/libpjproject-x86_64-x64-vc14-Release-Dynamic.lib
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.microsiptp)
endif()
set_target_properties(microsiptp PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
    QT_TARGET_COMPANY_NAME "freeorxx@sina.com"
    QT_TARGET_RC_ICONS "${CMAKE_CURRENT_SOURCE_DIR}/res/image/logo.ico"
    QT_TARGET_PRODUCT_NAME "microsiptp"
    QT_TARGET_DESCRIPTION "micro sip telephone"
    QT_TARGET_VERSION "0.1.0.0"
    QT_TARGET_COPYRIGHT "freeorxx@sina.com"
)
if(WIN32)
    _qt_internal_generate_win32_rc_file(${PROJECT_NAME})
endif()

include(GNUInstallDirs)
install(TARGETS microsiptp
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(microsiptp)
endif()
