cmake_minimum_required(VERSION 3.24)

project(dxf2jeo VERSION 1.0.3 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)

include(GoogleTest)
enable_testing()

find_package(cxxopts REQUIRED)
find_package(fmt REQUIRED)
find_package(jsoncons REQUIRED)
find_package(libdxfrw REQUIRED)
find_package(GTest REQUIRED)

configure_file(src/Dxf2JeoVersion.h.in Dxf2JeoVersion.h)

add_library(libdxf2jeo STATIC)
target_sources(libdxf2jeo
    PUBLIC
        src/ArcUtils.h
        src/Dxf2Jeo.h
        src/DxfColors.h
        src/DxfModel.h
        src/DxfReader.h
        src/DxfWriter.h
        src/Jeo2Dxf.h
        src/JeoModel.h
        src/JeoReader.h
        src/JeoWriter.h
    PRIVATE
        src/ArcUtils.cpp
        src/Dxf2Jeo.cpp
        src/DxfColors.cpp
        src/DxfReader.cpp
        src/DxfWriter.cpp
        src/Jeo2Dxf.cpp
        src/JeoReader.cpp
        src/JeoWriter.cpp
)
target_include_directories(libdxf2jeo PUBLIC src)
target_link_libraries(libdxf2jeo PRIVATE fmt::fmt jsoncons libdxfrw::libdxfrw)

add_executable(dxf2jeo)
target_sources(dxf2jeo PRIVATE src/Dxf2JeoExe.cpp)
target_include_directories(dxf2jeo PRIVATE "${PROJECT_BINARY_DIR}")
target_link_libraries(dxf2jeo PRIVATE cxxopts::cxxopts fmt::fmt libdxf2jeo)

add_executable(jeo2dxf)
target_sources(jeo2dxf PRIVATE src/Jeo2DxfExe.cpp)
target_include_directories(jeo2dxf PRIVATE "${PROJECT_BINARY_DIR}")
target_link_libraries(jeo2dxf PRIVATE cxxopts::cxxopts fmt::fmt libdxf2jeo)

add_executable(dxf2jeo_tests)
target_sources(dxf2jeo_tests PRIVATE test/Dxf2JeoTests.cpp)
target_link_libraries(dxf2jeo_tests PRIVATE libdxf2jeo gtest::gtest)
target_compile_definitions(dxf2jeo_tests PRIVATE "TEST_ASSET_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/test/asset\"")
gtest_discover_tests(dxf2jeo_tests DISCOVERY_MODE PRE_TEST DISCOVERY_TIMEOUT 30 WORKING_DIRECTORY $<TARGET_FILE_DIR:dxf2jeo_tests>)

install(TARGETS dxf2jeo)
install(TARGETS jeo2dxf)