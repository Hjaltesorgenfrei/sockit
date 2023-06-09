#pragma once
#include <cstdint>
#include <cassert>

enum PacketType : uint8_t { 
    PACKET_TYPE_CONNECTION_REQUEST = 2,
    PACKET_TYPE_CONNECTION_ACCEPTED,
    PACKET_TYPE_CONNECTION_DENIED,
    PACKET_TYPE_PRINT,
    PACKET_TYPE_HEARTBEAT,
};

const char * packet_type_string( uint8_t packet_type )
{
    switch ( packet_type )
    {
        case PACKET_TYPE_CONNECTION_REQUEST:                return "connection request";
        case PACKET_TYPE_CONNECTION_ACCEPTED:               return "connection accepted";
        case PACKET_TYPE_CONNECTION_DENIED:                 return "connection denied";
        case PACKET_TYPE_PRINT:                             return "print";
        case PACKET_TYPE_HEARTBEAT:                         return "heartbeat";
        default:
            assert( false );
            return "???";
    }
}

#pragma pack(push, 1)
struct Packet {
    PacketType type;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct PacketConnectionRequest : Packet {
    uint64_t client_guid;
    uint16_t connect_sequence;

    PacketConnectionRequest() {
        type = PacketType::PACKET_TYPE_CONNECTION_REQUEST;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct PacketConnectionAccepted : Packet {
    uint64_t client_guid;
    uint16_t connect_sequence;

    PacketConnectionAccepted() {
        type = PacketType::PACKET_TYPE_CONNECTION_ACCEPTED;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct PacketConnectionDenied : Packet {
    uint64_t client_guid;
    uint16_t connect_sequence;

    PacketConnectionDenied() {
        type = PacketType::PACKET_TYPE_CONNECTION_DENIED;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct PacketPrint : Packet {
    char data[256];

    PacketPrint() {
        type = PacketType::PACKET_TYPE_PRINT;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct PacketHeartbeat : Packet {
    PacketHeartbeat() {
        type = PacketType::PACKET_TYPE_HEARTBEAT;
    }
};
#pragma pack(pop)
