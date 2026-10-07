cmake_minimum_required(VERSION 3.8)
project(p_controller_plugin)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()
set(CMAKE_CXX_STANDARD 17)

find_package(ament_cmake REQUIRED)

# Nav2 may not be installed (e.g. on the dev machine). Skip this package instead of
# failing the whole workspace build.
find_package(nav2_core QUIET)
if(NOT nav2_core_FOUND)
  message(WARNING "nav2_core not found - skipping p_controller_plugin. "
                  "Install ros-$ENV{ROS_DISTRO}-navigation2 to build it.")
  ament_package()
  return()
endif()

find_package(rclcpp REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(visualization_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(pluginlib REQUIRED)
find_package(nav2_util REQUIRED)
find_package(nav2_costmap_2d REQUIRED)

set(deps rclcpp rclcpp_lifecycle geometry_msgs nav_msgs visualization_msgs
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
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
  # TODO: ament_add_gtest(test_p_controller test/test_p_controller.cpp)
endif()

ament_export_include_directories(include)
ament_export_libraries(p_controller)
ament_export_dependencies(${deps})
ament_package()
