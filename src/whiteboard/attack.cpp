/* $Id$ */
/*
 Copyright (C) 2010 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
 Part of the Battle for Wesnoth Project http://www.wesnoth.org

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 2
 or at your option any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY.

 See the COPYING file for more details.
 */

/**
 * @file attack.cpp
 */

#include "attack.hpp"

#include "visitor.hpp"

#include "arrow.hpp"
#include "unit.hpp"

namespace wb
{

attack::attack(unit& subject, const map_location& target_hex, const map_location& source_hex, const map_location& dest_hex,
		arrow_ptr arrow, fake_unit_ptr fake_unit)
	: move(subject, source_hex, dest_hex, arrow, fake_unit)
	, target_hex_(target_hex)
{

}

attack::~attack()
{
}

void attack::accept(visitor& v)
{
	v.visit_attack(boost::static_pointer_cast<attack>(shared_from_this()));
}

} // end namespace wb
