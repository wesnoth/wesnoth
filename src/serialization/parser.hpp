/*
	Copyright (C) 2005 - 2025
	by Philippe Plantier <ayin@anathas.org>
	Copyright (C) 2005 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
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

#pragma once

#include "config.hpp"
#include "configr_assign.hpp"
#include "serialization/tokenizer.hpp"

class abstract_validator;

namespace io
{
config read(std::istream& in, abstract_validator* validator = nullptr);      // Throws config::error
config read(const std::string& in, abstract_validator* validator = nullptr); // Throws config::error
config read_gz(std::istream& in, abstract_validator* validator = nullptr);
config read_bz2(std::istream& in, abstract_validator* validator = nullptr);

void write(std::ostream& out, const configr_of& cfg, unsigned int level = 0);
void write_gz(std::ostream& out, const configr_of& cfg);
void write_bz2(std::ostream& out, const configr_of& cfg);
void write_key_val(std::ostream& out,
		const std::string& key,
		const config::attribute_value& value,
		unsigned level,
		std::string& textdomain);
void write_open_child(std::ostream& out, const std::string& child, unsigned int level);
void write_close_child(std::ostream& out, const std::string& child, unsigned int level);

} // namespace io
