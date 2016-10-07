//
// Created by Eric Dattore on 10/5/16.
//

#ifndef DUG_STRUCTURES_H
#define DUG_STRUCTURES_H

#include <cstdint>

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

#endif //DUG_STRUCTURES_H
