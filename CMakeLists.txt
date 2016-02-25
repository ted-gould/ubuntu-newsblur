project(ubuntu-newsblur CXX)
cmake_minimum_required(VERSION 2.8.9)

find_package(Qt5Core)
find_package(Qt5Qml)
find_package(Qt5Quick)
# find_package(ubuntu-sdk-libs)

# Automatically create moc files
set(CMAKE_AUTOMOC ON)

# Components PATH
execute_process(
    COMMAND dpkg-architecture -qDEB_HOST_MULTIARCH
    OUTPUT_VARIABLE ARCH_TRIPLET
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(QT_IMPORTS_DIR "lib/${ARCH_TRIPLET}")

add_subdirectory(qml)

# Build the app
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(newsblur
feeds.cpp
feeds.h
main.cpp
newsblurconnection.cpp
newsblurconnection.h
qtquick2applicationviewer.cpp
stories.cpp
stories.h
)

qt5_use_modules(newsblur Gui Qml Quick)

install(TARGETS newsblur DESTINATION ${CMAKE_BINARY_DIR})

