#ifndef PROTOCOL_RDT_CHECKSUM_H
#define PROTOCOL_RDT_CHECKSUM_H

#include <cstdint>
#include <cstddef>
#include "protocol/rdt_header.h"

class RdtChecksum {
public:
    // Calculates a 32-bit checksum for the given header and payload.
    // The checksum field in the header should be 0 before calling this.
    static uint32_t calculate(const RdtHeader& header, const char* payload, size_t payloadLen);

    // Verifies if the checksum of the received header and payload matches.
    static bool verify(const RdtHeader& header, const char* payload, size_t payloadLen);
};
#endif // PROTOCOL_RDT_CHECKSUM_H
