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
#ifndef SERIALIZATION_PARSER_HPP_INCLUDED
#define SERIALIZATION_PARSER_HPP_INCLUDED

#include <iosfwd>
#include <vector>

class config;
struct line_source;

line_source get_line_source(std::vector< line_source > const &line_src, int line);

//read data in, clobbering existing data.
void read(config &cfg, std::istream &in, std::vector< line_source > const *lines = 0); //throws config::error

std::string write(config const &cfg);

#endif
