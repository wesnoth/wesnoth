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
 * @file attack.hpp
 */

#ifndef ATTACK_HPP_
#define ATTACK_HPP_

#include "move.hpp"

#include "typedefs.hpp"

#include <boost/enable_shared_from_this.hpp>

namespace wb
{

class attack: public move
{
	friend class validate_visitor;
	friend class highlight_visitor;

public:
	attack(unit& subject, const map_location& target_hex, const map_location& source_hex, const map_location& dest_hex,
			arrow_ptr arrow, fake_unit_ptr fake_unit);
	virtual ~attack();

	virtual void accept(visitor& v);

	virtual bool execute();

	/** Gets called by display when drawing a hex, to allow actions to draw to the screen. */
	virtual void draw_hex(const map_location& hex);

	map_location const& get_target_hex() const {return target_hex_; }

private:
		///the target of the attack
		map_location target_hex_;
};

} // end namespace wb

/** Dumps an attack on a stream, for debug purposes. */
std::ostream &operator<<(std::ostream &s, wb::attack const& attack);

#endif /* ATTACK_HPP_ */
