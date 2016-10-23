//
// Created by Eric Dattore on 10/6/16.
//

#ifndef DUG_FUNCTIONS_H
#define DUG_FUNCTIONS_H

#include "structures.h"

// TODO: make functions to build different types of queries (i.e. A, CNAME, NS, MX, PTR, SOA)
// TODO: make functions to read different types of answers (i.e. A, CNAME, NS, MX, PTR, SOA)
// TODO: add debug messages
void make_a_record_request();

uint8_t *make_request(uint8_t*);

void read_a_record_response();

void make_query_header(DNSQueryHeader *);

void make_query_question(DNSQueryQuestion *);

// TODO: write directly to buffer
uint8_t *domain_to_dns_format(std::string);

void close_socket(int);

std::string read_name(uint8_t*, size_t);

bool is_pointer(uint8_t);

std::string decode_answer_type(uint16_t);

std::string decode_ip(uint32_t);

#endif // DUG_FUNCTIONS_H
