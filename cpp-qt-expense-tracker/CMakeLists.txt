cmake_minimum_required(VERSION 3.16)
project(expense_tracker_gui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)

add_executable(expense_tracker_gui
    main.cpp
    MainWindow.h
    MainWindow.cpp
)

target_link_libraries(expense_tracker_gui PRIVATE Qt6::Widgets Qt6::Sql)
