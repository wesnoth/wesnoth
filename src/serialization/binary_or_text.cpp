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

//! @file serialization/binary_or_text.cpp 
//! Read/Write file in binary (compressed) or text-format (uncompressed).

#include "global.hpp"

#include "binary_or_text.hpp"
#include "config.hpp"
#include "filesystem.hpp"
#include "serialization/binary_wml.hpp"
#include "serialization/parser.hpp"
#include "serialization/preprocessor.hpp"

#include <sstream>

#include <boost/bind.hpp>
#include <boost/iostreams/filter/gzip.hpp>

bool detect_format_and_read(config &cfg, std::istream &in, std::string* error_log)
{
	unsigned char c = in.peek();
	if (c < 4) {
		read_compressed(cfg, in);
		return true;
	} else {
		read(cfg, in, error_log);
		return false;
	}
}

void write_possibly_compressed(std::ostream &out, config &cfg, bool compress)
{
	if (compress)
		write_compressed(out, cfg);
	else
		write(out, cfg);
}

config_writer::config_writer(
	std::ostream &out, bool compress, const std::string &textdomain,int level) :
		filter_(),
		out_ptr_(compress ? &filter_ : &out), //ternary indirection creates a temporary
		out_(*out_ptr_), //now MSVC will allow binding to the reference member
		compress_(compress), 
		level_(0), 
		textdomain_(textdomain)
{
	if(compress_) {
		if (level >=0)
			filter_.push(boost::iostreams::gzip_compressor(boost::iostreams::gzip_params(level)));
		else
			filter_.push(boost::iostreams::gzip_compressor(boost::iostreams::gzip_params()));

		filter_.push(out);
	}
}


void config_writer::write_define(const preproc_map::value_type& def)
{

	out_ << "#line " << def.second.linenum << " " << def.second.location << "\n";
	out_ << "#textdomain " << def.second.textdomain << "\n";
	out_ << "#define " << def.first;
	for(std::vector<std::string>::const_iterator it = def.second.arguments.begin();
				  it != def.second.arguments.end();
				  ++it)
	{
		out_ << *it << " ";
	}
	out_ << "\n";
	out_ << def.second.value;
	out_ << "#enddef\n";
}

void config_writer::write(const preproc_map& defines_map)
{
	std::for_each(defines_map.begin(), defines_map.end(), boost::bind(&config_writer::write_define,this,_1));
}

void config_writer::write(const config &cfg)
{
	::write(out_, cfg, level_);
}

void config_writer::write_child(const std::string &key, const config &cfg)
{
	open_child(key);
	::write(out_, cfg, level_);
	close_child(key);
}

void config_writer::write_key_val(const std::string &key, const std::string &value)
{
	::write_key_val(out_, key, value, level_, textdomain_);
}

void config_writer::open_child(const std::string &key)
{
	::write_open_child(out_, key, level_++);
}

void config_writer::close_child(const std::string &key)
{
	::write_close_child(out_, key, --level_);
}

bool config_writer::good() const
{
	return out_.good();
}

