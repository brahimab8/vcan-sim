#include "driver.h"

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <utility>

#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

SocketCanDriver::SocketCanDriver(std::string interface_name)
    : interface_name_(std::move(interface_name))
{
    // Create a raw CAN socket using the PF_CAN protocol family.
    socket_fd_ = ::socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (socket_fd_ < 0) {
        return;
    }

    struct ifreq ifr {};
    std::strncpy(ifr.ifr_name, interface_name_.c_str(), IFNAMSIZ - 1);

    if (::ioctl(socket_fd_, SIOCGIFINDEX, &ifr) < 0) {
        // Failed to look up the interface index; clean up the socket and
        // mark the driver as unusable.
        ::close(socket_fd_);
        socket_fd_ = -1;
        return;
    }

    struct sockaddr_can addr {};
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    // Bind the socket to the requested CAN interface so reads/writes target
    // that interface. On failure, close the socket and set an invalid fd.
    if (::bind(socket_fd_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        ::close(socket_fd_);
        socket_fd_ = -1;
    }
}

SocketCanDriver::~SocketCanDriver()
{
    // Close the underlying socket descriptor if it was successfully opened.
    if (socket_fd_ >= 0) {
        ::close(socket_fd_);
    }
}

bool SocketCanDriver::send(const CanFrame& frame)
{
    // Validate driver state and DLC
    if (socket_fd_ < 0 || frame.dlc > CAN_MAX_DLC) {
        return false;
    }

    // Populate a native `struct can_frame` for the socket write. We mask the
    // ID to SFF (standard frame format) bits.
    struct can_frame raw_frame {};
    raw_frame.can_id = static_cast<canid_t>(frame.id & CAN_SFF_MASK);
    raw_frame.len = frame.dlc;

    // Copy only the used bytes from the fixed-size data buffer
    std::copy_n(frame.data.begin(), frame.dlc, raw_frame.data);

    // Write the entire can_frame structure to the socket. A full-size write
    // indicates success.
    const ssize_t written = ::write(socket_fd_, &raw_frame, sizeof(raw_frame));
    return written == static_cast<ssize_t>(sizeof(raw_frame));
}

bool SocketCanDriver::receive(CanFrame& frame)
{
    // Ensure the socket is open
    if (socket_fd_ < 0) {
        return false;
    }

    // Read a native can_frame from the socket. A short read indicates an
    // error or partial read.
    struct can_frame raw_frame {};
    const ssize_t read_size = ::read(socket_fd_, &raw_frame, sizeof(raw_frame));
    if (read_size != static_cast<ssize_t>(sizeof(raw_frame))) {
        return false;
    }

    // Determine whether the frame uses extended IDs and extract the ID
    // accordingly. Both EFF (extended) and SFF (standard) supported.
    const bool is_extended = (raw_frame.can_id & CAN_EFF_FLAG) != 0;
    frame.id = is_extended
        ? static_cast<uint32_t>(raw_frame.can_id & CAN_EFF_MASK)
        : static_cast<uint32_t>(raw_frame.can_id & CAN_SFF_MASK);

    // Clip DLC to the maximum allowed and copy payload bytes into the
    // fixed-size `frame.data` buffer, zeroing the remainder for determinism.
    frame.dlc = std::min<uint8_t>(raw_frame.len, CAN_MAX_DLC);
    frame.data.fill(0);
    std::copy_n(raw_frame.data, frame.dlc, frame.data.begin());

    return true;
}
