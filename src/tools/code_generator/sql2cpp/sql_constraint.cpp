/*
	Copyright (C) 2013 by Pierre Talbot <ptalbot@mopong.net>
	Part of the Battle for Wesnoth Project http://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "tools/code_generator/sql2cpp/sql_constraint.hpp"

namespace sql{
namespace constraint{

foreign_key::foreign_key(const std::string& name, const std::vector<std::string>& keys, const sql::ast::key_references& refs)
: base(name)
, keys(keys)
, refs(refs)
{}

}} // namespace sql::constraint
