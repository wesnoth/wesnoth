
/*
   Copyright (C) 2009 - 2017 by Yurii Chernyi <terraninfo@terraninfo.net>
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
 * AI Support engine - creating specific ai components from config
 * @file
 */

#ifndef AI_COMPOSITE_ENGINE_HPP_INCLUDED
#define AI_COMPOSITE_ENGINE_HPP_INCLUDED

#include "ai/composite/component.hpp"
#include "ai/contexts.hpp"

#include <algorithm>
#include <iterator>

//============================================================================

namespace ai {

class rca_context;
class ai_context;
class component;

class engine : public component {
public:
	engine( readonly_context &context, const config &cfg );


	virtual ~engine();

	virtual bool is_ok() const;

	static void parse_aspect_from_config( readonly_context &context, const config &cfg, const std::string &id, std::back_insert_iterator<std::vector< aspect_ptr > > b );


	static void parse_goal_from_config( readonly_context &context, const config &cfg, std::back_insert_iterator<std::vector< goal_ptr > > b );


	static void parse_candidate_action_from_config( rca_context &context, const config &cfg, std::back_insert_iterator<std::vector< candidate_action_ptr > > b );


	static void parse_engine_from_config( readonly_context &context, const config &cfg, std::back_insert_iterator<std::vector< engine_ptr > > b );


	static void parse_stage_from_config( ai_context &context, const config &cfg, std::back_insert_iterator<std::vector< stage_ptr > > b );


	//do not override that method in subclasses which cannot create aspects
	virtual void do_parse_aspect_from_config( const config &cfg, const std::string &id, std::back_insert_iterator< std::vector< aspect_ptr> > b );


	//do not override that method in subclasses which cannot create candidate_actions
	virtual void do_parse_candidate_action_from_config( rca_context &context, const config &cfg, std::back_insert_iterator<std::vector< candidate_action_ptr > > b );

	//do not override that method in subclasses which cannot create goals
	virtual void do_parse_goal_from_config( const config &cfg, std::back_insert_iterator<std::vector< goal_ptr > > b );

	//do not override that method in subclasses which cannot create engines
	virtual void do_parse_engine_from_config( const config &cfg, std::back_insert_iterator<std::vector< engine_ptr > > b );


	//do not override that method in subclasses which cannot create stages
	virtual void do_parse_stage_from_config( ai_context &context, const config &cfg, std::back_insert_iterator<std::vector< stage_ptr > > b );

	//do not override that method in subclasses which cannot evaluate formulas
	virtual std::string evaluate(const std::string& str);

	readonly_context& get_readonly_context();

	/**
	 * set ai context (which is not available during early initialization)
	 */
	virtual void set_ai_context(ai_context_ptr context);

	virtual ai_context_ptr get_ai_context();
	/**
	 * serialize
	 */
	virtual config to_config() const;


	virtual std::string get_id() const
	{ return id_; }

	virtual std::string get_engine() const
	{ return engine_; }

	virtual std::string get_name() const
	{ return name_; }

protected:
	readonly_context &ai_;
	ai_context_ptr ai_context_;

	/** name of the engine which has created this engine*/
	std::string engine_;
	std::string id_;
	std::string name_;
};


class engine_factory;

class engine_factory{
	bool is_duplicate(const std::string &name);
public:
	typedef std::shared_ptr< engine_factory > factory_ptr;
	typedef std::map<std::string, factory_ptr> factory_map;
	typedef std::pair<const std::string, factory_ptr> factory_map_pair;

	static factory_map& get_list() {
		static factory_map *engine_factories;
		if (engine_factories==nullptr) {
			engine_factories = new factory_map;
		}
		return *engine_factories;
	}

	virtual engine_ptr get_new_instance( readonly_context &ai, const config &cfg ) = 0;
	virtual engine_ptr get_new_instance( readonly_context &ai, const std::string& name ) = 0;

	engine_factory( const std::string &name )
	{
		if (is_duplicate(name)) {
			return;
		}
		factory_ptr ptr_to_this(this);
		get_list().insert(make_pair(name,ptr_to_this));
	}

	virtual ~engine_factory() {}
};


template<class ENGINE>
class register_engine_factory : public engine_factory {
public:
	register_engine_factory( const std::string &name )
		: engine_factory( name )
	{
	}

	virtual engine_ptr get_new_instance( readonly_context &ai, const config &cfg ){
		engine_ptr e = engine_ptr(new ENGINE(ai,cfg));
		if (!e->is_ok()) {
			return engine_ptr();
		}
		return e;
	}

	virtual engine_ptr get_new_instance( readonly_context &ai, const std::string& name ){
		config cfg;
		cfg["name"] = name;
		cfg["engine"] = "cpp"; // @Crab: what is the purpose of this line(neph)
		return engine_ptr(new ENGINE(ai,cfg));
	}
};

} //end of namespace ai

#endif
