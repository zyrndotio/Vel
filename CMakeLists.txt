cmake_minimum_required(VERSION 3.22)
project(vel VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(vel
    src/main.cpp
)

target_include_directories(vel PRIVATE src)

# Compiler warnings
if (MSVC)
    target_compile_options(vel PRIVATE /W4)
else()
    target_compile_options(vel PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Install the compiler and the files needed to use Vel after packaging.
include(GNUInstallDirs)
install(TARGETS vel RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                       BUNDLE DESTINATION .
                       LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES README.md LICENSE DESTINATION ${CMAKE_INSTALL_DOCDIR})
install(DIRECTORY docs/ DESTINATION ${CMAKE_INSTALL_DOCDIR})
install(DIRECTORY examples/ DESTINATION share/vel/examples)

# CPack selects a native package format for each supported host platform.
set(CPACK_PACKAGE_NAME "Vel")
set(CPACK_PACKAGE_VENDOR "Zyrndotio")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Vel Systems Programming Language")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Vel")

if(WIN32)
    set(CPACK_GENERATOR "WIX")
elseif(APPLE)
    set(CPACK_GENERATOR "productbuild")
else()
    set(CPACK_GENERATOR "DEB;RPM")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Zyrndotio")
endif()

include(CPack)
