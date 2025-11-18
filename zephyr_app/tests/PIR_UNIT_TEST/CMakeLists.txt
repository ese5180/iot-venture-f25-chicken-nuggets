cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(pir_test)

target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

# Add test source
target_sources(app PRIVATE src/pir_test.c)

# Add PIR driver source
target_sources(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/pir.c
)
