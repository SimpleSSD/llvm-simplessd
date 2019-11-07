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

add_compile_options(-g)
add_compile_options(-Wall)
add_compile_options(-Wextra)
add_compile_options(-Werror)
add_compile_options(-fno-rtti)

if (DEBUG_BUILD)
  add_compile_options(-O0)
  add_definitions(-DDEBUG_MODE)
else ()
  add_compile_options(-O2)
endif ()

set(SRC_BLOCK_COLLECTOR
  ./src/basic_block_collector.cc
)
set(SRC_INST_APPLIER
  ./src/instruction_applier.cc
)
set(SRC_STAT_GENERATOR
  ./src/stat_generator.cc
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

add_dependencies(llvm-simplessd inststat-generator)
