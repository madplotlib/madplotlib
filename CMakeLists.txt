cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 14)

project(madplotlib)

find_package(Qt5 REQUIRED COMPONENTS Charts)
find_package(Eigen3 REQUIRED)

add_executable(eigen_test eigen_tests.cpp)
target_link_libraries(eigen_test Qt5::Charts Eigen3::Eigen)