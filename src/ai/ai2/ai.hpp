/* $Id$ */
/*
   Copyright (C) 2003 - 2010 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file ai/ai2/ai.hpp
 * Stub AI
 */

#ifndef AI_AI2_AI_HPP_INCLUDED
#define AI_AI2_AI_HPP_INCLUDED

#include "../interface.hpp"
#include "../contexts.hpp"

#ifdef _MSC_VER
#pragma warning(push)
//silence "inherits via dominance" warnings
#pragma warning(disable:4250)
#endif

namespace ai {

class ai2 : public readwrite_context_proxy, public interface {
public:
	ai2(readwrite_context &context, const config &cfg)
		: cfg_(cfg),recursion_counter_(context.get_recursion_count())
	{
		init_readwrite_context_proxy(context);
	}
	virtual ~ai2() {}
	virtual void play_turn() {}
	virtual void switch_side(side_number side){
		set_side(side);
	}


	int get_recursion_count() const{
		return recursion_counter_.get_count();
	}


	virtual void new_turn()
	{
	}


	virtual config to_config() const{
		return config();
	}

private:
	const config &cfg_;
	recursion_counter recursion_counter_;

};

} //of namespace ai

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif

