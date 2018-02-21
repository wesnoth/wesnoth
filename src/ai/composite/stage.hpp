/*
   Copyright (C) 2009 - 2018 by Yurii Chernyi <terraninfo@terraninfo.net>
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
 * Composite AI stages
 */

#pragma once

#include "ai/composite/component.hpp"
#include "ai/composite/contexts.hpp"

namespace ai {

class ai_composite;

class stage : public virtual ai_context_proxy, public component {
public:

	/**
	 * Constructor
	 */
	stage( ai_context &context, const config &cfg );

	/**
	 * Initialization
	 */
	virtual	void on_create();


	/**
	 * Destructor
	 */
	virtual ~stage();

	/**
	 * Play the turn - strategy
	 * @return true only if game state has changed. Returning false is always safe.
	 */
	bool play_stage();


	/**
	 * get the value of the recursion counter
	 */
	int get_recursion_count() const;


	/**
	 * serialize
	 */
	virtual config to_config() const;

	virtual std::string get_id() const;
	virtual std::string get_name() const;
	virtual std::string get_engine() const;

protected:
	/**
	 * Play the turn - implementation
	 * @return true only if game state has changed. Returning false is always safe.
	 */
	virtual bool do_play_stage() = 0;

	recursion_counter recursion_counter_;

	config cfg_;

};


class idle_stage : public stage {
public:
	idle_stage( ai_context &context, const config &cfg );

	~idle_stage();

	virtual bool do_play_stage();
};


class stage_factory{
	bool is_duplicate(const std::string &name);
public:
	typedef std::shared_ptr< stage_factory > factory_ptr;
	typedef std::map<std::string, factory_ptr> factory_map;
	typedef std::pair<const std::string, factory_ptr> factory_map_pair;

	static factory_map& get_list() {
		static factory_map *stage_factories;
		if (stage_factories==nullptr) {
			stage_factories = new factory_map;
		}
		return *stage_factories;
	}

	virtual stage_ptr get_new_instance( ai_context &context, const config &cfg ) = 0;

	stage_factory( const std::string &name )
	{
		if (is_duplicate(name)) {
			return;
		}
		factory_ptr ptr_to_this(this);
		get_list().emplace(name,ptr_to_this);
	}

	virtual ~stage_factory() {}
};


template<class STAGE>
class register_stage_factory : public stage_factory {
public:
	register_stage_factory( const std::string &name )
		: stage_factory( name )
	{
	}

	virtual stage_ptr get_new_instance( ai_context &context, const config &cfg ){
		stage_ptr a(new STAGE(context,cfg));
		a->on_create();
		return a;
	}
};

} //end of namespace ai
