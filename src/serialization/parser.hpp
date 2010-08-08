/* $Id$ */
/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2010 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file */

#ifndef SERIALIZATION_PARSER_HPP_INCLUDED
#define SERIALIZATION_PARSER_HPP_INCLUDED

#include "global.hpp"
#include "config.hpp"

// Read data in, clobbering existing data.
void read(config &cfg, std::istream &in); 	// Throws config::error
void read(config &cfg, std::string &in); 	// Throws config::error
void read_gz(config &cfg, std::istream &in);

void write(std::ostream &out, config const &cfg, unsigned int level=0);
void write_key_val(std::ostream &out, const std::string &key, const config::attribute_value &value, unsigned level, std::string &textdomain);
void write_open_child(std::ostream &out, const std::string &child, unsigned int level);
void write_close_child(std::ostream &out, const std::string &child, unsigned int level);

#endif
