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

#include "config.hpp"
#include "serialization/binary_wml.hpp"
#include "serialization/parser.hpp"

bool detect_format_and_read(config &cfg, std::string const &data)
{
	try {
		compression_schema schema;
		read_compressed(cfg, data, schema);
		return true;
	} catch (config::error &) {
	}

	read(cfg, data);
	return false;
}
