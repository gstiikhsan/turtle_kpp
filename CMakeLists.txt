cmake_minimum_required(VERSION 3.5)
project(turtle_kpp)

set(CMAKE_CXX_STANDARD 17)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(turtlesim REQUIRED)

add_executable(pose_listener src/pose_listener.cpp)
ament_target_dependencies(pose_listener rclcpp turtlesim)

add_executable(ampera_draw src/ampera_draw.cpp)
ament_target_dependencies(ampera_draw rclcpp geometry_msgs turtlesim)

install(TARGETS
  pose_listener
  ampera_draw
  DESTINATION lib/${PROJECT_NAME})

ament_package()
