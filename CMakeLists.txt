cmake_minimum_required(VERSION 3.22)
project(vel VERSION 0.3.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(vel
    src/main.cpp
)

target_include_directories(vel PRIVATE src)

include(CTest)
if(BUILD_TESTING AND UNIX)
    add_test(NAME vel_frontend_smoke
             COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/smoke.sh $<TARGET_FILE:vel>)
    add_test(NAME vel_continue_and_targets
             COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/continue_tests.sh $<TARGET_FILE:vel>)
    add_test(NAME vel_type_checker
             COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/type_checker_tests.sh $<TARGET_FILE:vel>)
    add_test(NAME vel_data_types
             COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/data_type_tests.sh $<TARGET_FILE:vel>)
    add_test(NAME vel_cli_commands
             COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/cli_commands.sh $<TARGET_FILE:vel>)
    add_test(NAME vel_source_tests
             COMMAND $<TARGET_FILE:vel> test ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    add_test(NAME vel_project_manifest
             COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/project_manifest.sh $<TARGET_FILE:vel>)
endif()

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
install(DIRECTORY assets/ DESTINATION share/vel/assets)
install(DIRECTORY docs/ DESTINATION ${CMAKE_INSTALL_DOCDIR})
install(DIRECTORY examples/ DESTINATION share/vel/examples)
install(DIRECTORY templates/ DESTINATION share/vel/templates)

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
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Zyrndotio")
    find_program(RPMBUILD_EXECUTABLE rpmbuild)
    if(RPMBUILD_EXECUTABLE)
        set(CPACK_GENERATOR "DEB;RPM")
    else()
        set(CPACK_GENERATOR "DEB")
        message(STATUS "rpmbuild not found; CPack will generate a Debian package only")
    endif()
endif()

include(CPack)
