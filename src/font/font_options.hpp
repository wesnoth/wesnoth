/*
	Copyright (C) 2015 - 2025
	by Iris Morelle <shadowm2006@gmail.com>
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

namespace font
{

enum class family_class
{
	sans_serif,
	monospace,
	script,
};

inline family_class decode_family_class(const std::string& str)
{
	if(str == "monospace") {
		return family_class::monospace;
	} else if(str == "script") {
		return family_class::script;
	} else {
		return family_class::sans_serif;
	}
}

} // end namespace font
