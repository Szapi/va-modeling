cmake_minimum_required(VERSION 3.10.0)

project(node_y VERSION 0.1.0 LANGUAGES C CXX)
add_executable(node_y main.cpp)
set_property(TARGET node_y PROPERTY CXX_STANDARD 23)
target_compile_options(node_y PUBLIC -ffast-math -Wall -Wextra -Wno-strict-aliasing -Ofast -ftree-vectorize -march=native -funroll-loops -fvect-cost-model=unlimited)

include_directories(../Utils/)
include_directories(../NumMethods/)
include_directories(../Extern/)
include_directories(../Defs/)
