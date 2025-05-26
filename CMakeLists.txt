cmake_minimum_required(VERSION 3.30...4.0)

project(incerr)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)


add_library(incerr STATIC)
add_library(incerr::incerr ALIAS incerr)

set(INCERR_SRC
    incerr.cpp)
list(TRANSFORM INCERR_SRC PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/src/)

target_sources(incerr PRIVATE ${INCERR_SRC})
target_sources(incerr
    PUBLIC
        FILE_SET pub_headers
        TYPE HEADERS
        BASE_DIRS
                include/)
target_sources(incerr
    PRIVATE
        FILE_SET priv_headers
        TYPE HEADERS
        BASE_DIRS
                src/private_inc/)
                
target_compile_features(incerr PUBLIC cxx_std_23)