/*
   Copyright (C) 2013 by Pierre Talbot <ptalbot@mopong.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef UMCD_OTL_HPP
#define UMCD_OTL_HPP

// You must install unixODBC: sudo apt-get install unixodbc-dev

#define OTL_ODBC
#define OTL_ODBC_UNIX
// Enable the use of the STL
#define OTL_STL

#ifdef HAVE_CXX11
   #define OTL_CPP_11_ON
#endif

#include "umcd/otl/otlv4.h"

#endif // UMCD_OTL_HPP
