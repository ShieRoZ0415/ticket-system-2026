cmake_minimum_required(VERSION 3.28)

project(Ticket_System_2026_main CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(.)

add_executable(Ticket_System_2026_main
        ticket.cpp
        BPT.h
)