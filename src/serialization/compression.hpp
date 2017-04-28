/*
   Copyright (C) 2014 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef COMPRESSION_HPP_INCLUDED
#define COMPRESSION_HPP_INCLUDED

#include <string>

namespace compression {
	enum format {
		NONE,
		GZIP,
		BZIP2
	};

	inline std::string format_extension(format compression_format)
	{
		switch(compression_format) {
		case GZIP:
			return ".gz";
		case BZIP2:
			return ".bz2";
		case NONE:
			return "";
		}
		return "";
	}
}

#endif
