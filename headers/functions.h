//
// Created by Eric Dattore on 10/6/16.
//

#ifndef DUG_FUNCTIONS_H
#define DUG_FUNCTIONS_H

#include "structures.h"

uint8_t *send_and_recv(std::string const &, std::string const &);

void make_query_header(DNSQueryHeader *);

void make_query_question(DNSQueryQuestion *);

uint8_t *domain_to_dns_format(std::string);

void close_socket(int);

std::string read_name(uint8_t *, size_t);

bool is_pointer(uint8_t);

std::string decode_answer_type(uint16_t);

std::string decode_ip(uint32_t);

void decode_header(DNSQueryHeader *, uint16_t);

std::string get_dns_error(uint16_t);

#endif // DUG_FUNCTIONS_H
