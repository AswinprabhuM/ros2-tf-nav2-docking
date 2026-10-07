cmake_minimum_required(VERSION 3.8)
project(docking_comparison)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()
set(CMAKE_CXX_STANDARD 17)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(docking_interfaces REQUIRED)

foreach(exe service_server service_client action_server action_client)
  add_executable(${exe} src/${exe}.cpp)
  ament_target_dependencies(${exe} rclcpp rclcpp_action docking_interfaces)
  install(TARGETS ${exe} DESTINATION lib/${PROJECT_NAME})
endforeach()

install(DIRECTORY launch DESTINATION share/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
