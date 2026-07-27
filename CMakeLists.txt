cmake_minimum_required(VERSION 3.10)
project(StartLens VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INSTALL_PREFIX /usr)

# ===== أضف Network إلى المكونات =====
find_package(Qt5 COMPONENTS Widgets Network REQUIRED)
find_package(X11 REQUIRED)

set(SOURCES
    main.cpp
    startlens.cpp
    startlens_scanner.cpp
    startlens_usage.cpp
    startlens_view.cpp
    startlens_settings.cpp
    startlens_settingsdialog.cpp
    shutdowndialog.cpp
    applistmodel.cpp
    carddelegate.cpp
    qhotkey.cpp
)

set(HEADERS
    startlens.h
    startlens_scanner.h
    startlens_usage.h
    startlens_view.h
    startlens_settings.h
    startlens_settingsdialog.h
    shutdowndialog.h
    SafeFileWriter.h
    applistmodel.h
    carddelegate.h
    qhotkey.h
    utils.h
)

add_executable(startlens ${SOURCES} ${HEADERS})

# ===== أضف Qt5::Network إلى الروابط =====
target_link_libraries(startlens PRIVATE Qt5::Widgets Qt5::Network X11::X11)

# تثبيت البرنامج والأيقونات والصور
install(TARGETS startlens RUNTIME DESTINATION bin)
install(FILES resources/startlens.desktop DESTINATION share/applications)
install(FILES resources/icons/startlens.svg DESTINATION share/icons/hicolor/scalable/apps RENAME startlens.svg)
install(DIRECTORY resources/IMG/ DESTINATION share/startlens/images
        FILES_MATCHING PATTERN "*.jpg" PATTERN "*.png" PATTERN "*.jpeg" PATTERN "*.svg")
