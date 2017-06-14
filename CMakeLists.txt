cmake_minimum_required(VERSION 3.0)
project(madplotlib)

include_directories(${CMAKE_CURRENT_LIST_DIR})

find_package(Qt5 REQUIRED COMPONENTS Charts)
include_directories(${Qt5_Charts_INCLUDE_DIRS})

find_package(Eigen3 REQUIRED)
include_directories(${Eigen3_INCLUDE_DIRS})

add_executable(eigen_test eigen_tests.cpp)
target_link_libraries(eigen_test Qt5::Charts)