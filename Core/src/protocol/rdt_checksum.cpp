#include "protocol/rdt_checksum.h"

// Calculates a 32-bit checksum for the given header and payload.
uint32_t RdtChecksum::calculate(const RdtHeader& header, const char* payload, size_t payloadLen) {
    uint32_t sum = 0;
    
    // Sum header fields
    sum += header.seq_num;
    sum += header.ack_num;
    sum += header.payload_len;
    sum += header.flags;
    
    // Sum payload bytes
    for (size_t i = 0; i < payloadLen; ++i) {
        sum += static_cast<uint8_t>(payload[i]);
    }
    
    return sum;
}

// Verifies if the checksum of the received header and payload matches.
bool RdtChecksum::verify(const RdtHeader& header, const char* payload, size_t payloadLen) {
    return calculate(header, payload, payloadLen) == header.checksum;
}
