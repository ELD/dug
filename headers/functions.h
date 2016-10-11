//
// Created by Eric Dattore on 10/6/16.
//

#ifndef DUG_FUNCTIONS_H
#define DUG_FUNCTIONS_H

#include "structures.h"

DNSQueryHeader *make_query_header();

DNSQueryQuestion *make_query_question();

uint8_t *domain_to_dns_format(std::string);

void close_socket(int);

uint16_t get_domain_offset_from_answer(uint16_t);

#endif // DUG_FUNCTIONS_H
