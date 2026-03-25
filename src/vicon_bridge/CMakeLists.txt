cmake_minimum_required(VERSION 3.16)
project(vicon_bridge)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(tf2 REQUIRED)                    
find_package(tf2_geometry_msgs REQUIRED) 

# Include directories
include_directories(include)

# Add publisher executable
add_executable(vicon_bridge_node src/vicon_bridge_node.cpp src/Vicon_packet_reader.cpp)
ament_target_dependencies(vicon_bridge_node
  rclcpp
  geometry_msgs
  tf2
  tf2_geometry_msgs
)

# Install executables
install(TARGETS 
  vicon_bridge_node
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
