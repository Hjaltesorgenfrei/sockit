#pragma once

enum class PacketType : char {
    PRINT = 0,
    PRINT_ACK = 1,
};

struct Packet {
    PacketType type;
    uint32_t length; 
    char data[1024];
};