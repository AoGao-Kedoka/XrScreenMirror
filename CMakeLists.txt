cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(
	XRLib
    GIT_REPOSITORY git@github.com:AoGao-Kedoka/XRLib.git
	GIT_TAG 4ee6587e76a066ca908384da50ebe54310845042
 )
FetchContent_MakeAvailable(XRLib)

if (UNIX)
    find_package(X11 REQUIRED)
    include_directories(${X11_INCLUDE_DIR})

    find_path(XRANDR_INCLUDE_DIR X11/extensions/Xrandr.h)
    find_library(XRANDR_LIBRARY Xrandr)
    if (NOT XRANDR_INCLUDE_DIR OR NOT XRANDR_LIBRARY)
        message(FATAL_ERROR "Xrandr library not found. Ensure libxrandr-dev is installed.")
    endif()
endif()

add_executable(XrScreenMirror
    main.cpp
    ICapture.h
    LinuxCapture.cpp
    LinuxCapture.h
    CustomPass.h
    CustomPass.cpp
    WindowsCapture.h
    WindowsCapture.cpp)

target_link_libraries(XrScreenMirror PRIVATE XRLib::XRLib ${X11_LIBRARIES} ${XRANDR_LIBRARY})

add_custom_target(copy_resources ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/resources/"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/resources"
        "${PROJECT_BINARY_DIR}/resources"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/shaders/"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/shaders"
        "${PROJECT_BINARY_DIR}/shaders"
)
