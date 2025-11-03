cmake_minimum_required(VERSION 3.30...4.0)

project(incerr)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

include(cmake/CPM.cmake)
CPMAddPackage("gh:Neargye/magic_enum@0.9.7")


add_library(incerr INTERFACE)
add_library(incerr::incerr ALIAS incerr)
target_link_libraries(incerr INTERFACE magic_enum::magic_enum)

target_sources(incerr
    PUBLIC
        FILE_SET pub_headers
        TYPE HEADERS
        BASE_DIRS
                include)
                
target_compile_features(incerr INTERFACE cxx_std_23)