//
// Created by Eric Dattore on 10/6/16.
//

#ifndef DUG_FUNCTIONS_H
#define DUG_FUNCTIONS_H

#include "structures.h"

DNSQueryHeader *make_query_header();
DNSQueryQuestion *make_query_question(std::string const&);

#endif //DUG_FUNCTIONS_H
