cmake_minimum_required(VERSION 3.5...4.0)

project(argless VERSION 2.0.0 LANGUAGES CXX)

option(ARGLESS_SINGLE_HEADER "Use single header." ON)
option(ARGLESS_ENABLE_MODULE "Build C++ modules support." OFF)
option(ARGLESS_INSTALL "Install CMake targets during install step." ${PROJECT_IS_TOP_LEVEL})

option(ARGLESS_EXAMPLE "Build Example." OFF)
option(ARGLESS_TEST "Build unit tests." OFF)

include(GNUInstallDirs)

set(ARGLESS_CONFIG_INSTALL_DIR "${CMAKE_INSTALL_DATADIR}/cmake/${PROJECT_NAME}" CACHE INTERNAL "")
set(ARGLESS_INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_INCLUDEDIR}")
set(ARGLESS_TARGETS_EXPORT_NAME "${PROJECT_NAME}Targets")
set(ARGLESS_CMAKE_VERSION_CONFIG_FILE "${PROJECT_NAME}ConfigVersion.cmake")
set(ARGLESS_CMAKE_PROJECT_CONFIG_FILE "${PROJECT_NAME}Config.cmake")

if (NOT ARGLESS_SINGLE_HEADER)
    include(FetchContent)
    find_package(tetter QUIET)
    if (tetter_FOUND)
        message(STATUS "Found: tetter ${tetter_VERSION}")
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

if (ARGLESS_SINGLE_HEADER)
    set(ARGLESS_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/single_include")
else()
    set(ARGLESS_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
    target_link_libraries(${PROJECT_NAME} INTERFACE tetter::tetter)
endif()

set(ARGLESS_SOURCE_DIR "${PROJECT_SOURCE_DIR}/module_src")

target_include_directories(${PROJECT_NAME} INTERFACE
    $<BUILD_INTERFACE:${ARGLESS_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:${ARGLESS_INCLUDE_INSTALL_DIR}>
)

if (ARGLESS_ENABLE_MODULE)
    add_library(${PROJECT_NAME}-module OBJECT)
    add_library(${PROJECT_NAME}::${PROJECT_NAME}-module ALIAS ${PROJECT_NAME}-module)
    target_sources(${PROJECT_NAME}-module PUBLIC FILE_SET CXX_MODULES
        BASE_DIRS ${ARGLESS_SOURCE_DIR}
        FILES "${ARGLESS_SOURCE_DIR}/argless.cppm"
    )
    target_compile_features(${PROJECT_NAME}-module PUBLIC cxx_std_20 cxx_std_23)
    target_link_libraries(${PROJECT_NAME}-module PRIVATE ${PROJECT_NAME})
endif()

if (ARGLESS_EXAMPLE)
    add_subdirectory(example)
endif()

if (ARGLESS_TEST)
    message(WARNING "ARGLESS_TEST is not yet implemented")
endif()

if (ARGLESS_INSTALL)
    install(
        DIRECTORY "${ARGLESS_INCLUDE_DIR}/"
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

    if (ARGLESS_ENABLE_MODULE)
        install(
            TARGETS ${PROJECT_NAME}-module
            EXPORT ${ARGLESS_TARGETS_EXPORT_NAME}
            FILE_SET CXX_MODULES DESTINATION ${ARGLESS_INCLUDE_INSTALL_DIR}
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
