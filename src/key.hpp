/*
   Copyright (C) 2003 - 2016 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef KEY_HPP_INCLUDED
#define KEY_HPP_INCLUDED

#include <boost/cstdint.hpp>

/**
 * Class that keeps track of all the keys on the keyboard.
 * Whether any key is pressed or not can be found by using its
 * operator[]. Note though that it is generally better to use
 * key events to see when keys are pressed rather than to poll using
 * this object.
 */
class CKey
{
	const boost::uint8_t *key_list;

public:
	CKey();
	bool operator[](int k) const;
};

#endif
