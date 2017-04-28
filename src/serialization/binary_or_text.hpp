/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2017 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
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

#include "config.hpp"
#include "preprocessor.hpp"
#include "serialization/compression.hpp"
#include "serialization/parser.hpp"

#include <boost/iostreams/filtering_stream.hpp>

/** Class for writing a config out to a file in pieces. */
class config_writer
{
public:
	config_writer(std::ostream &out, compression::format compress);
	config_writer(std::ostream &out, bool compress, int level = -1);
	/** Default implementation, but defined out-of-line for efficiency reasons. */
	~config_writer();

	void write(const config &cfg);

	void write_child(const std::string &key, const config &cfg);
	void open_child(const std::string &key);
	void close_child(const std::string &key);
	bool good() const;

	/// This template function will work with any type that can be assigned to
	/// an attribute_value.
	template <typename T>
	void write_key_val(const std::string &key, const T &value)
	{
		config::attribute_value v;
		v = value;
		::write_key_val(out_, key, v, level_, textdomain_);
	}

private:
	boost::iostreams::filtering_stream<boost::iostreams::output> filter_;
	std::ostream *out_ptr_;
	std::ostream &out_;
	compression::format compress_;
	unsigned int level_;
	std::string textdomain_;
};

#endif
