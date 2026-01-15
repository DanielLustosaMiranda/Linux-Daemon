cmake_minimum_required(VERSION 3.12)
project(daemon-min C)

find_package(PkgConfig REQUIRED)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

pkg_check_modules(UDEV REQUIRED libudev)

add_executable(${PROJECT_NAME} src/daemon.c)

target_link_libraries(${PROJECT_NAME} PRIVATE ${UDEV_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${UDEV_INCLUDE_DIRS})
