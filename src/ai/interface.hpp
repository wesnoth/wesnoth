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

/**
 * @file
 * Interface to the AI.
 */

#ifndef AI_INTERFACE_HPP_INCLUDED
#define AI_INTERFACE_HPP_INCLUDED

#include <boost/shared_ptr.hpp>         // for shared_ptr
#include <global.hpp>
#include <map>                          // for map, map<>::value_compare
#include <string>                       // for string, basic_string, etc
#include <utility>                      // for make_pair, pair
#include "savegame_config.hpp"       // for savegame_config
#include "config.hpp"             // for config
#include "ai/game_info.hpp"             // for ai_ptr, side_number
namespace ai { class ai_context; }

namespace ai {

class interface : savegame::savegame_config {
public:
	/**
	 * The constructor.
	 */
	interface()
		: savegame_config()
	{
	}


	virtual ~interface() {}


	/**
	 * Function that is called when the AI must play its turn.
	 * Derived classes should implement their AI algorithm in this function.
	 */
	virtual void play_turn() = 0;

	/**
	 * Function called when a a new turn is played
	 */
	virtual void new_turn() = 0;

	/**
	 * Function called after the new ai is created
	 *
	 */
	virtual void on_create() {
	}

	virtual void switch_side(ai::side_number side) = 0;

        /** Evaluate */
        virtual std::string evaluate(const std::string& /*str*/)
			{ return "evaluate command not implemented by this AI"; }

	/** Describe self*/
	virtual std::string describe_self() const;


	/** serialize to config **/
	virtual config to_config() const = 0;
};

class ai_factory;

class ai_factory{
public:
	typedef boost::shared_ptr< ai_factory > factory_ptr;
	typedef std::map<std::string, factory_ptr> factory_map;
	typedef std::pair<const std::string, factory_ptr> factory_map_pair;

	static factory_map& get_list() {
		static factory_map *ai_factories;
		if (ai_factories==NULL) {
			ai_factories = new factory_map;
		}
		return *ai_factories;
	}

	virtual ai_ptr get_new_instance( ai_context &context, const config &cfg) = 0;

	ai_factory( const std::string &name )
	{
		factory_ptr ptr_to_this(this);
		get_list().insert(make_pair(name,ptr_to_this));
	}

	virtual ~ai_factory() {}
};


template<class AI>
class register_ai_factory : public ai_factory {
public:
	register_ai_factory( const std::string &name )
		: ai_factory( name )
	{
	}

	virtual ai_ptr get_new_instance( ai_context &context, const config &cfg){
		ai_ptr a(new AI(context,cfg));
		a->on_create();
		return a;
	}
};



} //end of namespace ai

#endif
