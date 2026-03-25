#include <tf2/LinearMath/Quaternion.h>

#include <chrono>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <rclcpp/rclcpp.hpp>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>

#include <map>
#include <cmath>
#include <algorithm> 

#include "../include/vicon_bridge/Vicon_packet_reader.h"

namespace
{
// renaming ros types.
// pose names:
using pose_msg = geometry_msgs::msg::PoseStamped;
using pose_pub = rclcpp::Publisher<pose_msg>::SharedPtr;

// for ros wall timer
using wall_timer = rclcpp::TimerBase::SharedPtr;

// adding ros types
using rclcpp::init;
using rclcpp::SensorDataQoS;
using rclcpp::shutdown;
using rclcpp::spin;
using tf2::Quaternion;

// adding std types
using std::bind;
using std::make_shared;
using std::chrono::milliseconds;
}  // namespace

class Vicon_bridge : public rclcpp::Node
{
   public:
    Vicon_bridge() : Node("vicon_bridge")
    {
        auto qos = SensorDataQoS();

        // Timer-based publishing at 100Hz (matching Vicon rate)
        timer_ = this->create_wall_timer(milliseconds(10),  // 100Hz = 10ms period
                                         bind(&Vicon_bridge::timer_callback, this));

        RCLCPP_INFO(this->get_logger(), "Vicon publisher started at 100Hz");
    }

   private:
    void timer_callback()
    {
        try
        {
            // Update Vicon objects with new position data
            std::vector<Vicon_object> objects = vicon_reader_.read();

            for (const auto& object : objects)
            {
                // Took these checks from the python code.
                if (std::isnan(object.x) || std::isnan(object.y) || std::isnan(object.z)) {
                    continue; 
                }
                if (object.x == 0.0 && object.y == 0.0 && object.z == 0.0) {
                    continue;
                }
                
                // create a new publisher if we run into a new one.
                if (publishers_.find(object.name) == publishers_.end())
                {
                    // Remove spaces from vicon topic
                    std::string topic_name = "vicon/" + object.name;
                    std::replace(topic_name.begin(), topic_name.end(), ' ', '_');
                    
                    publishers_[object.name] = this->create_publisher<pose_msg>(topic_name, SensorDataQoS());
                    RCLCPP_INFO(this->get_logger(), "New object on stream, publishing to: %s", topic_name.c_str());
                }

                auto msg = pose_msg();
                msg.header.stamp = this->get_clock()->now();
                msg.header.frame_id = "vicon";

                // Convert mm to meters
                msg.pose.position.x = object.x / 1000.0;
                msg.pose.position.y = object.y / 1000.0;
                msg.pose.position.z = object.z / 1000.0;

                // Convert degrees to quaternion angles
                Quaternion quat;
                quat.setRPY(object.roll * M_PI / 180.0,
                            object.pitch * M_PI / 180.0,
                            object.yaw * M_PI / 180.0);

                msg.pose.orientation = tf2::toMsg(quat);

                // Debug logging
                RCLCPP_DEBUG(this->get_logger(),
                            "Pos(%.3f, %.3f, %.3f)m, Rot(%.1f, %.1f, %.1f)deg",
                            msg.pose.position.x,
                            msg.pose.position.y,
                            msg.pose.position.z,
                            object.roll,
                            object.pitch,
                            object.yaw);

                // Create message with consistent timing
                publishers_[object.name]->publish(msg);
            }
        }
        catch (const std::exception &e)
        {
            RCLCPP_ERROR_THROTTLE(this->get_logger(),
                                  *this->get_clock(),
                                  1000,  // Log at most once per second
                                  "Error reading Vicon data: %s",
                                  e.what());
        }
    }

    Vicon_reader vicon_reader_;  // vicon reader object
    std::map<std::string, pose_pub> publishers_; // pose publisher map
    wall_timer timer_;           // wall timer for controlling publishing rate
};

int main(int argc, char **argv)
{
    init(argc, argv);
    auto node = make_shared<Vicon_bridge>();
    spin(node);
    shutdown();
    return 0;
}
