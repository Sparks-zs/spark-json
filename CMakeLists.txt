cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(spark-json LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(src)

add_library(spark_json
    STATIC
        src/spark_json.cpp
        src/spark_json.hpp
    )

add_executable(test test.cpp)

target_link_libraries(test
    PRIVATE
        spark_json)