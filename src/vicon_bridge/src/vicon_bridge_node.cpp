#include <tf2/LinearMath/Quaternion.h>

#include <chrono>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <rclcpp/rclcpp.hpp>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>

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
        pub_ = this->create_publisher<pose_msg>("vicon_pose", qos);

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
            // Update Vicon object with new position data
            Vicon_object object = vicon_reader_.read();

            // Create message with consistent timing
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

            pub_->publish(msg);
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
    pose_pub pub_;               // pose publisher
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