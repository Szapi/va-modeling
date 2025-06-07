cmake_minimum_required(VERSION 3.10.0)

project(logistic VERSION 0.1.0 LANGUAGES C CXX)
add_executable(logistic main.cpp)
set_property(TARGET logistic PROPERTY CXX_STANDARD 23)
target_compile_options(logistic PUBLIC -ffast-math)

include_directories(../Utils/)
include_directories(../NumMethods/)
include_directories(../Defs/)
