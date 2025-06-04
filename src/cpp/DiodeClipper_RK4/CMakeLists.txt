cmake_minimum_required(VERSION 3.10.0)

project(rk4_clipper VERSION 0.1.0 LANGUAGES C CXX)
add_executable(rk4_clipper main.cpp)
set_property(TARGET rk4_clipper PROPERTY CXX_STANDARD 23)
target_compile_options(rk4_clipper PUBLIC -ffast-math -Wall -Wextra -Wno-strict-aliasing)

include_directories(../Utils/)
include_directories(../NumMethods/)
include_directories(../Extern/)
include_directories(../Defs/)
