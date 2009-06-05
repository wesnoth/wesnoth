/* $Id$ */
/*
   Copyright (C) 2009 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file ai/composite/stage.hpp
 * Composite AI stages
 */

#ifndef AI_COMPOSITE_STAGE_HPP_INCLUDED
#define AI_COMPOSITE_STAGE_HPP_INCLUDED

#include "../../global.hpp"

#include "contexts.hpp"
#include "../contexts.hpp"
#include "../../config.hpp"
#include <boost/shared_ptr.hpp>
#include <map>
#include <string>

namespace ai {

namespace composite_ai {

class ai_composite;

class stage : public virtual composite_ai_context_proxy {
public:

	/**
	 * Constructor
	 */
	stage( composite_ai_context &context, const config &cfg );

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
	 */
	void play_stage();


	/**
	 * get the value of the recursion counter
	 */
	int get_recursion_count() const;

protected:
	/**
	 * Play the turn - implementation
	 */
	virtual void do_play_stage() = 0;

	recursion_counter recursion_counter_;

	const config &cfg_;

};


class idle_stage : public stage {
public:
	idle_stage( composite_ai_context &context, const config &cfg );

	~idle_stage();

	virtual void do_play_stage();
};


class stage_factory;

class stage_factory{
public:
	typedef boost::shared_ptr< stage_factory > factory_ptr;
	typedef std::map<std::string, factory_ptr> factory_map;
	typedef std::pair<const std::string, factory_ptr> factory_map_pair;

	static factory_map& get_list() {
		static factory_map *stage_factories;
		if (stage_factories==NULL) {
			stage_factories = new factory_map;
		}
		return *stage_factories;
	}

	virtual stage_ptr get_new_instance( composite_ai_context &context, const config &cfg ) = 0;

	stage_factory( const std::string &name )
	{
		factory_ptr ptr_to_this(this);
		get_list().insert(make_pair(name,ptr_to_this));
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

	virtual stage_ptr get_new_instance( composite_ai_context &context, const config &cfg ){
		stage_ptr a(new STAGE(context,cfg));
		a->on_create();
		return a;
	}
};


} //end of namespace composite_ai

} //end of namespace ai

#endif
