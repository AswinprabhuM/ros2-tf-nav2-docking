cmake_minimum_required(VERSION 3.8)
project(p_controller_plugin)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()
set(CMAKE_CXX_STANDARD 17)

find_package(ament_cmake REQUIRED)

find_package(nav2_core REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(pluginlib REQUIRED)
find_package(nav2_util REQUIRED)
find_package(nav2_costmap_2d REQUIRED)

set(deps rclcpp rclcpp_lifecycle geometry_msgs nav_msgs
  tf2 tf2_ros tf2_geometry_msgs pluginlib nav2_core nav2_util nav2_costmap_2d)

add_library(p_controller SHARED src/p_controller.cpp)
target_include_directories(p_controller PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
ament_target_dependencies(p_controller ${deps})

# Registers plugin.xml so controller_server can discover the class.
pluginlib_export_plugin_description_file(nav2_core plugin.xml)

install(TARGETS p_controller
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)
install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY params DESTINATION share/${PROJECT_NAME})

if(BUILD_TESTING)
  # Individual linters instead of ament_lint_auto: no per-file copyright headers
  # (the repo has a top-level LICENSE), so copyright checks are left out.
  find_package(ament_cmake_cpplint REQUIRED)
  find_package(ament_cmake_uncrustify REQUIRED)
  ament_cpplint(FILTERS -legal/copyright)
  ament_uncrustify()
  find_package(ament_cmake_gtest REQUIRED)
  ament_add_gtest(test_p_controller test/test_p_controller.cpp)
  target_link_libraries(test_p_controller p_controller)
  ament_target_dependencies(test_p_controller ${deps})
endif()

ament_export_include_directories(include)
ament_export_libraries(p_controller)
ament_export_dependencies(${deps})
ament_package()
