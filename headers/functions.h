//
// Created by Eric Dattore on 10/6/16.
//

#ifndef DUG_FUNCTIONS_H
#define DUG_FUNCTIONS_H

#include "includes.h"

namespace po = boost::program_options;

std::pair<po::options_description, po::variables_map> make_command_line_parser(int, const char **);

uint8_t *send_and_recv(std::string const &, std::string const &, std::string const &);

void make_query_header(DNSQueryHeader *);

void make_query_question(DNSQueryQuestion *, std::string const &);

uint8_t *domain_to_dns_format(std::string);

void close_socket(int);

std::pair<std::string, int> read_name(uint8_t *, size_t);

std::string decode_answer_type(uint16_t);

std::string decode_ip(uint32_t);

void decode_header(DNSQueryHeader *, uint16_t);

std::string get_dns_error(uint16_t);

#endif // DUG_FUNCTIONS_H
