/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Copyright (C) 2005 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef SERIALIZATION_BINARY_OR_TEXT_HPP_INCLUDED
#define SERIALIZATION_BINARY_OR_TEXT_HPP_INCLUDED

#include <iosfwd>
#include <string>

class config;

//function which reads a file, and automatically detects whether it's compressed or not before
//reading it. If it's not a valid file at all, it will throw an error as if it was trying to
//read it as text WML. Returns true iff the format is compressed
bool detect_format_and_read(config &cfg, std::istream &in); //throws config::error

//function which writes a file, compressed or not depending on a flag
void write_possibly_compressed(std::string const &filename, config &cfg, bool compress);

#endif
