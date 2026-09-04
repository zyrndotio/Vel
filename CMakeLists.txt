cmake_minimum_required(VERSION 3.22)
set(VEL_VERSION "0.3.1" CACHE STRING "Vel release version")
project(vel VERSION ${VEL_VERSION} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(vel
    src/main.cpp
)

add_library(vel_std STATIC
    runtime/vel_std.cpp
)
target_include_directories(vel_std PUBLIC runtime)

target_include_directories(vel PRIVATE src)
target_compile_definitions(vel PRIVATE VEL_VERSION_STRING="${VEL_VERSION}")

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

if(BUILD_TESTING)
    add_executable(vel_std_runtime_smoke
        tests/runtime/vel_std_smoke.cpp
    )
    target_link_libraries(vel_std_runtime_smoke PRIVATE vel_std)
    add_test(NAME vel_std_runtime_smoke
             COMMAND $<TARGET_FILE:vel_std_runtime_smoke>)
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
install(TARGETS vel_std
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES runtime/vel_std.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vel)
install(FILES README.md LICENSE DESTINATION ${CMAKE_INSTALL_DOCDIR})
install(DIRECTORY assets/ DESTINATION share/vel/assets)
install(DIRECTORY docs/ DESTINATION ${CMAKE_INSTALL_DOCDIR})
install(DIRECTORY examples/ DESTINATION share/vel/examples)
install(DIRECTORY templates/ DESTINATION share/vel/templates)

# CPack selects a native package format for each supported host platform.
set(CPACK_PACKAGE_NAME "Vel")
set(CPACK_PACKAGE_VENDOR "Zyrndotio")
set(CPACK_PACKAGE_CONTACT "https://github.com/zyrndotio/Vel/issues")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/zyrndotio/Vel")
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
