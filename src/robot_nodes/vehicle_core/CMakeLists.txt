cmake_minimum_required(VERSION 3.8)
project(vehicle_core)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ─── Find dependencies ───────────────────────────────────────────────
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(gcs_interfaces REQUIRED)
find_package(builtin_interfaces REQUIRED)
find_package(std_msgs REQUIRED)

# ─── Telemetry Publisher Library ─────────────────────────────────────
add_library(telemetry_lib src/telemetry_publisher.cpp)
target_include_directories(telemetry_lib PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
ament_target_dependencies(telemetry_lib
  rclcpp
  rclcpp_lifecycle
  gcs_interfaces
  builtin_interfaces
  std_msgs)

# ─── Failsafe Manager Library ────────────────────────────────────────
add_library(failsafe_lib src/failsafe_manager.cpp)
target_include_directories(failsafe_lib PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
ament_target_dependencies(failsafe_lib
  rclcpp
  rclcpp_lifecycle
  gcs_interfaces
  builtin_interfaces
  std_msgs)

# ─── Executable Node ─────────────────────────────────────────────────
add_executable(telemetry_node src/telemetry_node_main.cpp)
target_link_libraries(telemetry_node telemetry_lib failsafe_lib)
ament_target_dependencies(telemetry_node
  rclcpp
  rclcpp_lifecycle
  gcs_interfaces)

install(TARGETS telemetry_node
  DESTINATION lib/${PROJECT_NAME})

install(TARGETS telemetry_lib failsafe_lib
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib)

install(DIRECTORY include/
  DESTINATION include)

# ─── BUILD_TESTING ───────────────────────────────────────────────────
if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)
  find_package(ament_cmake_pytest REQUIRED)
  find_package(launch_testing_ament_cmake REQUIRED)

  # ── C++ gtest: Telemetry Serialization ────────────────────────────
  ament_add_gtest(test_telemetry_node
    test/test_telemetry_node.cpp)
  target_link_libraries(test_telemetry_node telemetry_lib)
  ament_target_dependencies(test_telemetry_node
    rclcpp
    gcs_interfaces
    builtin_interfaces)

  # ── C++ gtest: Failsafe State Machine ─────────────────────────────
  ament_add_gtest(test_failsafe_state_machine
    test/test_failsafe_state_machine.cpp)
  target_link_libraries(test_failsafe_state_machine failsafe_lib)
  ament_target_dependencies(test_failsafe_state_machine
    rclcpp
    gcs_interfaces)

  # ── Python pytest: Launch Integration Test ────────────────────────
  add_launch_test(test/test_telemetry_launch.py
    TIMEOUT 60
    OUTPUT_FILE ${CMAKE_CURRENT_BINARY_DIR}/test_telemetry_launch.log)
endif()

ament_package()