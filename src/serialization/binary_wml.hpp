/* $Id$ */
/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2008 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/


#ifndef SERIALIZATION_BINARY_WML_HPP_INCLUDED
#define SERIALIZATION_BINARY_WML_HPP_INCLUDED

#include <iosfwd>
#include <map>
#include <string>

class config;

//! Holds the schema by which config objects can be compressed and decompressed.
struct compression_schema
{
	compression_schema() : char_to_word(), word_to_char() { }

	typedef std::map< unsigned int, std::string > char_word_map;
	char_word_map char_to_word;

	typedef std::map< std::string, unsigned int > word_char_map;
	word_char_map word_to_char;
};

//  Read and write compressed data. 

//! Write compressed data. 
//! The schema will be created and written with the data. 
//! However if you are making successive writes (e.g. a network connection),
//! you can re-use the same schema on the sending end, and the receiver can store 
//! the schema, meaning that the entire schema won't have to be transmitted each time.
void write_compressed(std::ostream &out, config const &cfg, compression_schema &schema);
//! Read compressed data. 
void read_compressed(config &cfg, std::istream &in, compression_schema &schema); //throws config::error

void write_compressed(std::ostream &out, config const &cfg);
void read_compressed(config &cfg, std::istream &in);

#endif
