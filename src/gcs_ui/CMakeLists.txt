cmake_minimum_required(VERSION 3.16)
project(gcs_ui)

# Find ROS2 toolset
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Quick)

# Enable automatic compilation handling for Qt6 Meta-Objects and UI layouts
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Source files
set(SOURCES
  src/main.cpp
  src/gcs_window.cpp
)

# Header files that require Qt MOC processing
set(HEADERS
  include/gcs_ui/gcs_window.hpp
)

# Define Executable
add_executable(gcs_main ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(gcs_main PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# Link ROS2 and Qt6 libraries
target_link_libraries(gcs_main
  rclcpp::rclcpp
  std_msgs::std_msgs
  Qt6::Core
  Qt6::Gui
  Qt6::Widgets
  Qt6::Quick
)

# Install directives for colcon workspace output
install(TARGETS gcs_main
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
