cmake_minimum_required(VERSION 3.10.0)

project(sparse_iv_table VERSION 0.1.0 LANGUAGES C CXX)
add_executable(sparse_iv_table main.cpp)
set_property(TARGET sparse_iv_table PROPERTY CXX_STANDARD 23)

target_compile_options(sparse_iv_table PUBLIC -ffast-math -Wall -Wextra -Wno-strict-aliasing -D TRM_ENABLE_DEBUG_MACROS=1 -Wno-unused -O3 -flto=auto)

include_directories(../Utils/)
include_directories(../NumMethods/)
include_directories(../Extern/)
include_directories(../Defs/)
