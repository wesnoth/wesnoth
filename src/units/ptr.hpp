/*
	Copyright (C) 2014 - 2024
	by Chris Beck <render787@gmail.com>
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

#include "utils/shared_reference.hpp"

#include <memory>

class unit;

typedef utils::shared_reference<unit> nonempty_unit_ptr;
typedef utils::shared_reference<const unit> nonempty_unit_const_ptr;
typedef std::shared_ptr<unit> unit_ptr;
typedef std::shared_ptr<const unit> unit_const_ptr;

// And attacks too!

class attack_type;

using attack_ptr = std::shared_ptr<attack_type>;
using const_attack_ptr = std::shared_ptr<const attack_type>;
