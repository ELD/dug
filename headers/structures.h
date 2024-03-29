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
#pragma pack(push, 1)
typedef struct {
    // Row 1
    uint16_t id : 16;
    // Row 2
    uint16_t qr : 1;
    uint16_t opcode : 4;
    uint16_t aa : 1;
    uint16_t tc : 1;
    uint16_t rd : 1;
    uint16_t ra : 1;
    uint16_t z : 3;
    uint16_t rcode : 4;
    // Row 3
    uint16_t qdcount : 16;
    // Row 4
    uint16_t ancount : 16;
    // Row 5
    uint16_t nscount : 16;
    // Row 6
    uint16_t arcount : 16;
} DNSQueryHeader;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
    uint16_t qtype : 16;
    uint16_t qclass : 16;
} DNSQueryQuestion;
#pragma pack(pop)

// Should be okay since name is typically c0 0c or byte 12
#pragma pack(push, 1)
typedef struct {
    uint16_t type : 16;
    uint16_t responseClass : 16;
    uint32_t ttl : 32;
    uint16_t rdlength : 16;
} DNSAnswerSegment;
#pragma pack(pop)

#endif // DUG_STRUCTURES_H
