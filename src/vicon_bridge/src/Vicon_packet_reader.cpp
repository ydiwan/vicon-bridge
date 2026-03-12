#include "../include/vicon_bridge/Vicon_packet_reader.h"

#include <math.h>  //for M_PI

#include <array>  //c++ arry for read()
#include <cstddef>
#include <iostream>

// libs for udp packets
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <strings.h>  //for bzero
#include <sys/socket.h>
#include <sys/types.h>
using namespace std;

#define PORT 51001  // this should never change

/// @brief Construction a udp listen socket to the vicon port number.
/// It will update the need Vicon member variable need to facilitate reading Vicon
/// UDP packets.
Vicon_reader::Vicon_reader()
{
    bzero(&servaddr, sizeof(servaddr));

    // Create a UDP Socket
    listenfd = socket(AF_INET,     // IPv4
                      SOCK_DGRAM,  // Datagram
                      0);          // Chose Automatically the Protocall
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);  // We Don't care about the IP addr
    servaddr.sin_port = htons(PORT);               // Use Vicon port
    servaddr.sin_family = AF_INET;

    // bind server address to socket file descriptor
    bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
}

/// @brief this will parse UDP packet and return the Vicon_object contain data for
/// the UDP packet.
/// @param buffer A std::byte buffer update by recvfrom() function
/// @return Vicon_object containing the new data from the UDP packet.
Vicon_object Vicon_reader::parse_data(std::byte *buffer)
{
    Vicon_object object;      // Vicon object
    std::byte *ptr = buffer;  // copy pointer address
    ptr += 4;                 // skip the first 4 bytes of packet header

    int8_t numItems =
        *((int8_t *)ptr);  // number of objects in the data packet to be read
    ptr += 4;              // move 4 bytes over

    ptr += 24;  // skip 24 bytes of string data

    // x position
    object.x = *((double *)ptr);
    ptr += 8;  // move 8 bytes over

    // y position
    object.y = *((double *)ptr);
    ptr += 8;  // move 8 bytes over

    // z position
    object.z = *((double *)ptr);
    ptr += 8;  // move 8 bytes over

    // Roll
    object.roll = (*((double *)ptr)) * (180.0 / M_PI);
    ptr += 8;  // move 8 bytes over

    // pitch
    object.pitch = (*((double *)ptr)) * (180.0 / M_PI);
    ptr += 8;  // move 8 bytes over

    // yaw
    object.yaw = (*((double *)ptr)) * (180.0 / M_PI);
    ptr += 8;  // move 8 bytes over

    return object;  // return new object
}

/// @brief Grabs a UDP packet calls the parse buffer, returns the vicon_object data.
/// @return Vicon_object contain the pose data.
Vicon_object Vicon_reader::read()
{
    std::byte buffer[MAX_BUF_SIZE];

    // receive message from Vicon server
    int num_bytes = recvfrom(
        this->listenfd, buffer, MAX_BUF_SIZE, 0, (struct sockaddr *)&servaddr, &len);

    if (num_bytes == -1)
    {
        // log recvfrom errors
        log_err();
    }

    return parse_data(buffer);
}
