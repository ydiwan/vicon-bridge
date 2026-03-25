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
#include <vector>
#include <cstring>
#include <algorithm>
#include <string>
using namespace std;

#define PORT 51001  // this should never change
#define SHIFT 75    // size of each vicon block

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
std::vector<Vicon_object> Vicon_reader::parse_data(std::byte *buffer)
{
    std::vector<Vicon_object> objects; // Vicon objects
    std::byte *ptr = buffer;  // copy pointer address
    ptr += 4;                 // skip the first 4 bytes of packet header

    int8_t numItems =
        *((int8_t *)ptr);  // number of objects in the data packet to be read
    ptr += 4;              // move 4 bytes over
    
    std::byte *data_start = buffer + 8;

    for(int i = 0; i < numItems; ++i){
        Vicon_object object;

        // Parse Name (24 bytes)    
        std::byte *item_ptr = data_start + (i * SHIFT);
            
        char name_buf[25] = {0}; // 1 extra byte for terminator char 
        std::memcpy(name_buf, item_ptr, 24);
        std::string obj_name(name_buf);

        // strip padding
        obj_name.erase(std::find(obj_name.begin(), obj_name.end(), '\0'), obj_name.end());
        object.name = obj_name;
        item_ptr += 24;  // move forward to position data

        // x position
        object.x = *((double *)item_ptr);
        item_ptr += 8;  // move 8 bytes over

        // y position
        object.y = *((double *)item_ptr);
        item_ptr += 8;  // move 8 bytes over

        // z position
        object.z = *((double *)item_ptr);
        item_ptr += 8;  // move 8 bytes over

        // Roll
        object.roll = (*((double *)item_ptr)) * (180.0 / M_PI);
        item_ptr += 8;  // move 8 bytes over

        // pitch
        object.pitch = (*((double *)item_ptr)) * (180.0 / M_PI);
        item_ptr += 8;  // move 8 bytes over

        // yaw
        object.yaw = (*((double *)item_ptr)) * (180.0 / M_PI);
        item_ptr += 8;  // move 8 bytes over

        objects.push_back(object);
    }
   
    return objects;  // return new object
}

/// @brief Grabs a UDP packet calls the parse buffer, returns the vicon_object data.
/// @return Vicon_object contain the pose data.
std::vector<Vicon_object> Vicon_reader::read()
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
