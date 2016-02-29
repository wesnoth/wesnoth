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

/** @file */

#ifndef AI_DEFAULT_AI_HPP_INCLUDED
#define AI_DEFAULT_AI_HPP_INCLUDED

#include "ai/interface.hpp"
#include "ai/composite/stage.hpp"

#ifdef _MSC_VER
#pragma warning(push)
//silence "inherits via dominance" warnings
#pragma warning(disable:4250)
#endif


namespace ai {

/** A trivial ai that sits around doing absolutely nothing. */
class idle_ai : public readwrite_context_proxy, public interface {
public:
	idle_ai(readwrite_context &context, const config& /*cfg*/);
	void play_turn();
	void new_turn();
	std::string describe_self() const;
	void switch_side(side_number side);
	int get_recursion_count() const;
	virtual config to_config() const;
private:
	recursion_counter recursion_counter_;
};

} //end of namespace ai

#ifdef _MSC_VER
#pragma warning(pop)
#endif


#endif
