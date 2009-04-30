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
 * @file ai/ai_interface.hpp
 * Interface to the AI.
 */

#ifndef AI_AI_INTERFACE_HPP_INCLUDED
#define AI_AI_INTERFACE_HPP_INCLUDED

#include "contexts.hpp"
#include "../formula_callable.hpp"

class ai_interface : public game_logic::formula_callable, public ai_readwrite_context {
public:
	/**
	 * The constructor.
	 */
	ai_interface(int side, bool master) : ai_readwrite_context(side,master) {
		add_ref(); //this class shouldn't be reference counted.
	}
	virtual ~ai_interface() {}

	/**
	 * Function that is called when the AI must play its turn.
	 * Derived classes should implement their AI algorithm in this function.
	 */
	virtual void play_turn() = 0;

	/**
	 * Function called when a a new turn is played
	 * Derived AIs should call this function each turn (expect first)
	 */
	virtual void new_turn() {
	}

protected:
	virtual void get_inputs(std::vector<game_logic::formula_input>* inputs) const;
	virtual variant get_value(const std::string& key) const;
};

#endif
