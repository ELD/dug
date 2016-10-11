//
// Created by Eric Dattore on 10/5/16.
//

#ifndef DUG_STRUCTURES_H
#define DUG_STRUCTURES_H

#include <cstdint>

// Layout:
// Header ~ 12 bytes
// QNAME ?? bytes, 63 bytes max?
// Question ~ 4 bytes
#pragma pack(1)
typedef struct {
    // Row 1
    uint16_t id;
    // Row 2
    uint8_t qr : 1;
    uint8_t opcode : 4;
    uint8_t aa : 1;
    uint8_t tc : 1;
    uint8_t rd : 1;
    uint8_t ra : 1;
    uint8_t z : 3;
    uint8_t rcode : 4;
    // Row 3
    uint16_t qdcount : 16;
    // Row 4
    uint16_t ancount : 16;
    // Row 5
    uint16_t nscount : 16;
    // Row 6
    uint16_t arcount : 16;
} DNSQueryHeader;

#pragma pack(1)
typedef struct {
    uint16_t qtype : 16;
    uint16_t qclass : 16;
} DNSQueryQuestion;

// Should be okay since name is typically c0 0c or byte 12
#pragma pack(1)
typedef struct {
    uint16_t nameOffset;
    uint16_t type;
    uint16_t responseClass;
    uint32_t ttl;
    uint16_t rdlength;
} DNSAnswerSegment;

// nameOffset is 0xc0XX
// mask with     0x3FFF

#endif // DUG_STRUCTURES_H
