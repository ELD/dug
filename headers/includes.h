//
// Created by Eric Dattore on 10/4/16.
//

#ifndef DUG_INCLUDES_H
#define DUG_INCLUDES_H

// C/C++ STL Headers
#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <sstream>
#include <boost/program_options.hpp>

// Unix headers
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

// Local includes
#include "constants.h"
#include "structures.h"
#include "functions.h"

namespace po = boost::program_options;

#endif //DUG_INCLUDES_H
