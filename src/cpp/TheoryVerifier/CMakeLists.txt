cmake_minimum_required(VERSION 3.10.0)

project(verifier VERSION 0.1.0 LANGUAGES C CXX)
add_executable(verifier main.cpp)
set_property(TARGET verifier PROPERTY CXX_STANDARD 23)
target_compile_options(verifier PUBLIC -ffast-math -Wall -Wextra -Wno-strict-aliasing -D TRM_ENABLE_DEBUG_MACROS=1)

include_directories(../Utils/)
include_directories(../NumMethods/)
include_directories(../Extern/)
include_directories(../Defs/)
