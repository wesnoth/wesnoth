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
public:
	attack(unit& subject, unit& target, const map_location& source_hex, const map_location& dest_hex, arrow_ptr arrow);
	virtual ~attack();

	virtual void accept(visitor& v);

private:
		///the target of the attack
		unit& target_;
};

}

#endif /* ATTACK_HPP_ */
