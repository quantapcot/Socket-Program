#ifndef PROTOCOL_RDT_HEADER_H
#define PROTOCOL_RDT_HEADER_H

#include <cstdint>

#define RDT_FLAG_ACK 0x01
#define RDT_FLAG_FIN 0x02
#define RDT_FLAG_SYN 0x04

// Struct RdtHeader: seq, ack, checksum, flags, len
#pragma pack(push, 1)
struct RdtHeader {
    uint32_t seq_num;     // Sequence number (0 or 1 for Stop-and-Wait)
    uint32_t ack_num;     // Acknowledgment number
    uint32_t checksum;    // Data and header integrity
    uint16_t payload_len; // Length of the payload in bytes
    uint8_t flags;        // Control flags (ACK, FIN, SYN)
};
#pragma pack(pop)
#endif // PROTOCOL_RDT_HEADER_H
