cmake_minimum_required(VERSION 3.5...4.0)

project(Example)

add_executable(${PROJECT_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
target_link_libraries(${PROJECT_NAME} PRIVATE argless)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
