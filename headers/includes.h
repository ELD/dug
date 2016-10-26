//
// Created by Eric Dattore on 10/4/16.
//

#ifndef DUG_INCLUDES_H
#define DUG_INCLUDES_H

// C/C++ STL Headers
#include <boost/program_options.hpp>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Unix headers
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// Local includes
#include "constants.h"
#include "structures.h"

namespace po = boost::program_options;

#endif // DUG_INCLUDES_H
