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

/**
 * @file
 * Read/Write file in binary (compressed) or text-format (uncompressed).
 */

#include "serialization/binary_or_text.hpp"
#include "config.hpp"
#include "log.hpp"
#include "wesconfig.h"
#include "serialization/parser.hpp"


#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/filter/gzip.hpp>

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)

config_writer::config_writer(
	std::ostream &out, compression::format compress) :
		filter_(),
		out_ptr_(compress ? &filter_ : &out), //ternary indirection creates a temporary
		out_(*out_ptr_), //now MSVC will allow binding to the reference member
		compress_(compress),
		level_(0),
		textdomain_(PACKAGE)
{
	if(compress_ == compression::GZIP) {
		filter_.push(boost::iostreams::gzip_compressor(boost::iostreams::gzip_params(9)));
		filter_.push(out);

	} else if(compress_ == compression::BZIP2) {
		filter_.push(boost::iostreams::bzip2_compressor(boost::iostreams::bzip2_params()));
		filter_.push(out);
	}
}
config_writer::config_writer(
	std::ostream &out, bool compress, int level) :
		filter_(),
		out_ptr_(compress ? &filter_ : &out), //ternary indirection creates a temporary
		out_(*out_ptr_), //now MSVC will allow binding to the reference member
		compress_(compress ? compression::GZIP : compression::NONE),
		level_(0),
		textdomain_(PACKAGE)
{
	if(compress_) {
		if (level >=0)
			filter_.push(boost::iostreams::gzip_compressor(boost::iostreams::gzip_params(level)));
		else
			filter_.push(boost::iostreams::gzip_compressor(boost::iostreams::gzip_params()));

		filter_.push(out);
	}
}

config_writer::~config_writer()
{
	//we only need this for gzip but we also do it for bz2 for unification.
	if(compress_ == compression::GZIP || compress_ == compression::BZIP2)
	{
		// prevent empty gz files because of https://svn.boost.org/trac/boost/ticket/5237
		out_ << "\n";
	}
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

