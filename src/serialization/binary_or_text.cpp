/*
	Copyright (C) 2005 - 2025
	by Guillaume Melquiond <guillaume.melquiond@gmail.com>
	Copyright (C) 2003 by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

#include "log.hpp"
#include "serialization/parser.hpp"
#include "wesconfig.h"

#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/filter/gzip.hpp>

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)

config_writer::config_writer(std::ostream& out, compression::format compress)
	: filter_()
	, out_ptr_(compress != compression::format::none ? &filter_ : &out) // ternary indirection creates a temporary
	, out_(*out_ptr_) // now MSVC will allow binding to the reference member
	, compress_(compress)
	, level_(0)
	, textdomain_(PACKAGE)
{
	if(compress_ == compression::format::gzip) {
		filter_.push(boost::iostreams::gzip_compressor(boost::iostreams::gzip_params(9)));
		filter_.push(out);
	} else if(compress_ == compression::format::bzip2) {
		filter_.push(boost::iostreams::bzip2_compressor(boost::iostreams::bzip2_params()));
		filter_.push(out);
	}
}

config_writer::config_writer(std::ostream& out, bool compress, int level)
	: filter_()
	, out_ptr_(compress ? &filter_ : &out) // ternary indirection creates a temporary
	, out_(*out_ptr_) // now MSVC will allow binding to the reference member
	, compress_(compress ? compression::format::gzip : compression::format::none)
	, level_(0)
	, textdomain_(PACKAGE)
{
	if(compress_ != compression::format::none) {
		if(level >= 0) {
			filter_.push(boost::iostreams::gzip_compressor(boost::iostreams::gzip_params(level)));
		} else {
			filter_.push(boost::iostreams::gzip_compressor(boost::iostreams::gzip_params()));
		}

		filter_.push(out);
	}
}

config_writer::~config_writer()
{
	// we only need this for gzip but we also do it for bz2 for unification.
	if(compress_ == compression::format::gzip || compress_ == compression::format::bzip2) {
		// prevent empty gz files because of https://svn.boost.org/trac/boost/ticket/5237
		out_ << "\n";
	}
}

void config_writer::write(const config& cfg)
{
	io::write(out_, cfg, level_);
}

void config_writer::write_child(const std::string& key, const config& cfg)
{
	open_child(key);
	io::write(out_, cfg, level_);
	close_child(key);
}

void config_writer::open_child(const std::string& key)
{
	io::write_open_child(out_, key, level_++);
}

void config_writer::close_child(const std::string& key)
{
	io::write_close_child(out_, key, --level_);
}

bool config_writer::good() const
{
	return out_.good();
}
