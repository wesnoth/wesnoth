/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@verizon.net>
   Copyright (C) 2005 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "config.hpp"
#include "filesystem.hpp"
#include "serialization/binary_wml.hpp"
#include "serialization/parser.hpp"

#include <sstream>

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
