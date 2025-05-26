cmake_minimum_required(VERSION 3.30...4.0)

project(incerr)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)


add_library(incerr INTERFACE)
add_library(incerr::incerr ALIAS incerr)

target_sources(incerr
    INTERFACE
        FILE_SET pub_headers
        TYPE HEADERS
        BASE_DIRS
                include/)
                
target_compile_features(incerr INTERFACE cxx_std_23)