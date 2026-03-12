#ifndef VICON_PACKET_READER
#define VICON_PACKET_READER

#include <errno.h>
#include <netinet/in.h>

#include <array>
#include <iostream>
#include <rclcpp/rclcpp.hpp>

#define MAX_BUF_SIZE 1024  // max Buffer size

struct Vicon_object
{
    double x;      // x position in mm
    double y;      // y position in mm
    double z;      // z position in mm
    double roll;   // roll axis in degrees
    double pitch;  // pitch axis in degrees
    double yaw;    // yaw axis in degrees

    /// @brief Vicon_object constructor set everything to zero.
    Vicon_object()
    {
        x = 0.0;
        y = 0.0;
        z = 0.0;
        roll = 0.0;
        pitch = 0.0;
        yaw = 0.0;
    };
};

class Vicon_reader
{
   private:
    int listenfd;                 // file descriptor for UDP socket
    socklen_t len;                // length of the socket
    struct sockaddr_in servaddr;  // socket address

   public:
    Vicon_reader();
    ~Vicon_reader();
    Vicon_object parse_data(std::byte *buffer);
    Vicon_object read();
};

/// @brief Helper function for Vicon_reader::read(). Logs ROS2 ERROR UDP socket read
/// errors.
inline void log_err()
{
    auto logger = rclcpp::get_logger("vicon_reader");

    switch (errno)
    {
        case EWOULDBLOCK:
            RCLCPP_ERROR(
                logger,
                "The socket's file descriptor is marked O_NONBLOCK and no data "
                "is waiting to be received; or MSG_OOB is set and no "
                "out-of-band data is available and either the socket's file "
                "descriptor is marked O_NONBLOCK or the socket does not support "
                "blocking to await out-of-band data");
            break;
        case EBADF:
            RCLCPP_ERROR(logger,
                         "The socket argument is not a valid file descriptor");
            break;
        case ECONNRESET:
            RCLCPP_ERROR(logger, "A connection was forcibly closed by a peer");
            break;
        case EFAULT:
            RCLCPP_ERROR(logger,
                         "The buffer parameter can not be accessed or written");
            break;
        case EINTR:
            RCLCPP_ERROR(logger, "The recv");
            break;
        case EINVAL:
            RCLCPP_ERROR(
                logger,
                "The MSG_OOB flag is set and no out-of-band data is available");
            break;
        case ENOTCONN:
            RCLCPP_ERROR(
                logger,
                "A receive is attempted on a connection-mode socket that is not "
                "connected");
            break;
        case ENOTSOCK:
            RCLCPP_ERROR(logger, "The socket argument does not refer to a socket");
            break;
        case EOPNOTSUPP:
            RCLCPP_ERROR(
                logger,
                "The specified flags are not supported for this socket type or "
                "protocol");
            break;
        case ETIMEDOUT:
            RCLCPP_ERROR(
                logger,
                "The connection timed out during connection establishment, or "
                "due to a transmission timeout on active connection");
            break;
        case EIO:
            RCLCPP_ERROR(
                logger,
                "An I/O error occurred while reading from or writing to the "
                "file system");
            break;
        case ENOBUFS:
            RCLCPP_ERROR(
                logger,
                "Insufficient resources were available in the system to perform "
                "the operation");
            break;
        case ENOMEM:
            RCLCPP_ERROR(logger,
                         "Insufficient memory was available to fulfill the request");
            break;
        case ENOSR:
            RCLCPP_ERROR(
                logger,
                "There were insufficient STREAMS resources available for the "
                "operation to complete");
            break;
    }
}

#endif