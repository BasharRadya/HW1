cmake_minimum_required(VERSION 3.17)
project(HW1)

set(CMAKE_CXX_STANDARD 11)

include_directories(.)

add_executable(HW1
        Commands.cpp
        Commands.h
        signals.cpp
        signals.h
        smash.cpp)
