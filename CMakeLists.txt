cmake_minimum_required(VERSION 3.5...4.0)

project(argless VERSION 2.0.0 LANGUAGES CXX)

set(MAIN_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(MAIN_PROJECT ON)
endif()

option(ARGLESS_SINGLE_HEADER "Use single header." ON)
option(ARGLESS_INSTALL "Install CMake targets during install step." ${MAIN_PROJECT})

if (NOT ARGLESS_SINGLE_HEADER)
    include(FetchContent)
    find_package(tetter QUIET)
    if (tetter_FOUND)
        message(STATUS "Found: tetter")
        set(TETTER_FETCHED OFF)
    else()
        message(STATUS "Not Found: tetter (will be downloaded)")
        set(TETTER_INSTALL ${ARGLESS_INSTALL})
        FetchContent_Declare(
            tetter
            GIT_REPOSITORY https://github.com/Maslowian/tetter.git
            GIT_TAG        master
        )
        FetchContent_MakeAvailable(tetter)
        set(TETTER_FETCHED ON)
    endif()
endif()

add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20) 

include(GNUInstallDirs)

set(ARGLESS_CONFIG_INSTALL_DIR "${CMAKE_INSTALL_DATADIR}/cmake/${PROJECT_NAME}" CACHE INTERNAL "")
set(ARGLESS_INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_INCLUDEDIR}")
set(ARGLESS_TARGETS_EXPORT_NAME "${PROJECT_NAME}Targets")
set(ARGLESS_CMAKE_VERSION_CONFIG_FILE "${PROJECT_NAME}ConfigVersion.cmake")
set(ARGLESS_CMAKE_PROJECT_CONFIG_FILE "${PROJECT_NAME}Config.cmake")

if (ARGLESS_SINGLE_HEADER)
    set(ARGLESS_INCLUDE_BUILD_DIR "${PROJECT_SOURCE_DIR}/single_include/")
else()
    set(ARGLESS_INCLUDE_BUILD_DIR "${PROJECT_SOURCE_DIR}/include/")
    target_link_libraries(${PROJECT_NAME} INTERFACE tetter::tetter)
endif()

target_include_directories(${PROJECT_NAME} INTERFACE
    $<BUILD_INTERFACE:${ARGLESS_INCLUDE_BUILD_DIR}>
    $<INSTALL_INTERFACE:${ARGLESS_INCLUDE_INSTALL_DIR}>
)

if (ARGLESS_INSTALL)
    install(
        DIRECTORY ${ARGLESS_INCLUDE_BUILD_DIR}
        DESTINATION ${ARGLESS_INCLUDE_INSTALL_DIR}
    )

    install(
        TARGETS ${PROJECT_NAME}
        EXPORT ${ARGLESS_TARGETS_EXPORT_NAME}
        INCLUDES DESTINATION ${ARGLESS_INCLUDE_INSTALL_DIR}
    )

    if (TETTER_FETCHED)
        install(
            TARGETS tetter
            EXPORT ${ARGLESS_TARGETS_EXPORT_NAME}
            INCLUDES DESTINATION ${ARGLESS_INCLUDE_INSTALL_DIR}
        )
    endif()

    install(
        EXPORT ${ARGLESS_TARGETS_EXPORT_NAME}
        FILE ${ARGLESS_CMAKE_PROJECT_CONFIG_FILE}
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${ARGLESS_CONFIG_INSTALL_DIR}
    )

    include(CMakePackageConfigHelpers)
    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/${ARGLESS_CMAKE_VERSION_CONFIG_FILE}"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )
    install(
        FILES "${CMAKE_CURRENT_BINARY_DIR}/${ARGLESS_CMAKE_VERSION_CONFIG_FILE}"
        DESTINATION ${ARGLESS_CONFIG_INSTALL_DIR}
    )
endif()
