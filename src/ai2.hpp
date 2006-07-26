/* $Id$ */
/*
   Copyright (C) 2003-5 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef AI2_HPP_INCLUDED
#define AI2_HPP_INCLUDED

#include "ai_interface.hpp"

class ai2 : public ai_interface
{
public:
	ai2(ai_interface::info& info) : ai_interface(info)
	{}
	virtual ~ai2() {}
	virtual void play_turn() {}

};


#endif
