/* $Id$ */
/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2011 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file */

#ifndef SERIALIZATION_BINARY_OR_TEXT_HPP_INCLUDED
#define SERIALIZATION_BINARY_OR_TEXT_HPP_INCLUDED

#include "preprocessor.hpp"

#include <boost/iostreams/filtering_stream.hpp>

class config;

/** Class for writing a config out to a file in pieces. */
class config_writer
{
public:
	config_writer(std::ostream &out, bool compress, int level = -1);
	/** Default implementation, but defined out-of-line for efficiency reasons. */
	~config_writer();

	void write(const config &cfg);

	void write_child(const std::string &key, const config &cfg);
	void write_key_val(const std::string &key, const std::string &value);
	void open_child(const std::string &key);
	void close_child(const std::string &key);
	bool good() const;

private:
	boost::iostreams::filtering_stream<boost::iostreams::output> filter_;
	std::ostream *out_ptr_;
	std::ostream &out_;
	bool compress_;
	unsigned int level_;
	std::string textdomain_;
};

#endif
