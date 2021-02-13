# SPDX-License-Identifier: GPL-3.0-or-later
#
# Copyright (C) 2019 CAMELab
#
# Author: Donghyun Gouk <kukdh1@camelab.org>
#

cmake_minimum_required(VERSION 3.10)
project(llvm-simplessd)

# Validate build type
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Select CMake build type." FORCE)

  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif ()

string (TOUPPER "${CMAKE_BUILD_TYPE}" uppercase_CMAKE_BUILD_TYPE)

if (CMAKE_BUILD_TYPE AND NOT uppercase_CMAKE_BUILD_TYPE MATCHES "^(DEBUG|RELEASE)$")
  message (FATAL_ERROR "Invalid value for CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
endif ()

# We requires LLVM library
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")

include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${PROJECT_SOURCE_DIR}
)

set(SRC_BLOCK_COLLECTOR
  ./src/basic_block_collector.cc
)
set(SRC_INST_APPLIER
  ./src/instruction_applier.cc
)
set(SRC_STAT_GENERATOR
  ./src/stat_generator.cc
  ./src/insts/insts.cc
  ./src/insts/arm/cortex_a57.cc
  ./src/insts/arm/cortex_r52.cc
)
set(SRC_UTIL
  ./src/util.cc
)

# LLVM Pass target
add_library(llvm-simplessd
  MODULE
  ${SRC_BLOCK_COLLECTOR}
  ${SRC_INST_APPLIER}
  ${SRC_UTIL}
)

# Statistic collector target
add_executable(inststat-generator
  ${SRC_STAT_GENERATOR}
)

target_compile_definitions(llvm-simplessd PRIVATE ${LLVM_DEFINITIONS})
target_compile_definitions(inststat-generator PRIVATE ${LLVM_DEFINITIONS})

target_compile_options(llvm-simplessd PRIVATE -g -fno-rtti)
target_compile_options(inststat-generator PRIVATE -g)

if (DEBUG_BUILD)
  target_compile_definitions(llvm-simplessd PRIVATE -DDEBUG_MODE)
  target_compile_definitions(inststat-generator PRIVATE -DDEBUG_MODE)
endif ()

add_dependencies(llvm-simplessd inststat-generator)
