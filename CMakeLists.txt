cmake_minimum_required(VERSION 3.10)
project(llvm_simplessd)

option(DEBUG_BUILD "Build LLVM Passes in debug mode." OFF)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

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
else ()
  add_compile_options(-O2)
endif ()

set(SRC_INST_STAT_GEN
  ./src/inst_stat_gen.cc
)

add_library(llvm_simplessd
  MODULE
  ${SRC_INST_STAT_GEN}
)
