//
// Created by Eric Dattore on 10/6/16.
//

#ifndef DUG_FUNCTIONS_H
#define DUG_FUNCTIONS_H

#include "structures.h"

void make_query_header(DNSQueryHeader *);

void make_query_question(DNSQueryQuestion *);

// TODO: write directly to buffer
uint8_t *domain_to_dns_format(std::string);

void close_socket(int);

std::string read_name(uint8_t*, size_t);

bool is_pointer(uint8_t);

std::string decode_answer_type(uint16_t);

#endif // DUG_FUNCTIONS_H
