# SPDX-License-Identifier: GPL-3.0-or-later
#
# Copyright (C) 2019 CAMELab
#
# Author: Donghyun Gouk <kukdh1@camelab.org>
#

cmake_minimum_required(VERSION 3.10)
project(llvm-simplessd)

# LLVM Pass debug mode
option(DEBUG_BUILD "Build LLVM Passes in debug mode." OFF)

# We requires LLVM library
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")

include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${PROJECT_SOURCE_DIR}
)

add_definitions(${LLVM_DEFINITIONS})

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

target_compile_options(llvm-simplessd PRIVATE -g -fno-rtti)
target_compile_options(inststat-generator PRIVATE -g)

if (DEBUG_BUILD)
  target_compile_options(llvm-simplessd PRIVATE -O0)
  target_compile_options(inststat-generator PRIVATE -O0)
  target_compile_definitions(llvm-simplessd PRIVATE -DDEBUG_MODE)
  target_compile_definitions(inststat-generator PRIVATE -DDEBUG_MODE)
else ()
  target_compile_options(llvm-simplessd PRIVATE -O2)
  target_compile_options(inststat-generator PRIVATE -O2)
endif ()

add_dependencies(llvm-simplessd inststat-generator)
