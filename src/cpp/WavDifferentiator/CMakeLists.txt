cmake_minimum_required(VERSION 3.10.0)

project(wavdiff VERSION 0.1.0 LANGUAGES C CXX)
add_executable(wavdiff main.cpp)
set_property(TARGET wavdiff PROPERTY CXX_STANDARD 23)

target_compile_options(wavdiff PUBLIC -ffast-math -Wall -Wextra -Wno-strict-aliasing -D TRM_ENABLE_DEBUG_MACROS=1 -Wno-unused -O3 -flto=auto)

include_directories(../Utils/)
include_directories(../NumMethods/)
include_directories(../Extern/)
include_directories(../Defs/)
