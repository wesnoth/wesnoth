/* $Id$ */
/*
   Copyright (C) 2003 - 2009 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file ai/ai2.hpp
 * AI-interface part 2, virtual.
 */

#ifndef AI_AI2_HPP_INCLUDED
#define AI_AI2_HPP_INCLUDED

#include "contexts.hpp"

class ai2 : public ai_readwrite_context
{
public:
	ai2(int side, bool master) : ai_readwrite_context(side, master)
	{}
	virtual ~ai2() {}
	virtual void play_turn() {}

};

#endif

