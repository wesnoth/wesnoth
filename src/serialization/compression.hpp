/*
	Copyright (C) 2014 - 2024
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include <string>

namespace compression
{
enum class format { none, gzip, bzip2 };

inline std::string format_extension(format compression_format)
{
	switch(compression_format) {
	case format::gzip:
		return ".gz";
	case format::bzip2:
		return ".bz2";
	case format::none:
		return "";
	}
	return "";
}
} // namespace compression
