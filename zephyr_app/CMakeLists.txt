# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(zephyr_app)

FILE(GLOB app_sources src/*.c)
# target_sources(app PRIVATE ${app_sources})
target_sources(app PRIVATE
    src/main.c
    src/sths34pf80_reg.c
)

target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

zephyr_library_sources(
    #src/sths34pf80_zephyr.c
    src/sths34pf80_reg.c
)