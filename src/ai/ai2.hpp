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

#include "ai_interface.hpp"
#include "contexts.hpp"

class ai2 : public ai::readwrite_context_proxy, public ai_interface {
public:
	ai2(ai::readwrite_context &context)
		: ai::side_context_proxy(context), ai::readonly_context_proxy(context), ai::readwrite_context_proxy(context),recursion_counter_(context.get_recursion_count())
	{}
	virtual ~ai2() {}
	virtual void play_turn() {}
	virtual void switch_side(ai::side_number side){
		set_side(side);
	}
	int get_recursion_count() const{
		return recursion_counter_.get_count();
	}
private:
	ai::recursion_counter recursion_counter_;

};

#endif

