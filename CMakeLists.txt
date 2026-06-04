cmake_minimum_required(VERSION 3.28)

project(Ticket_System_2026_main CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(.)

add_executable(Ticket_System_2026_main
        main.cpp
        BPT.h
        basic.h
        record.h
        store.h
        user.h
        train.h
        ticket.h
        order.h
        cmd.h
)