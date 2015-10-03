
/* $Id$ */
/*
   Copyright (C) 2009 - 2011 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * Composite AI with turn sequence which is a vector of stages
 * @file ai/composite/ai.cpp
 */

#include "ai.hpp"
#include "stage.hpp"
#include "../manager.hpp"
#include "../../foreach.hpp"
#include "../../log.hpp"

#include <boost/bind.hpp>
#include <boost/function.hpp>

namespace ai {

static lg::log_domain log_ai_composite("ai/composite");
#define DBG_AI_COMPOSITE LOG_STREAM(debug, log_ai_composite)
#define LOG_AI_COMPOSITE LOG_STREAM(info, log_ai_composite)
#define ERR_AI_COMPOSITE LOG_STREAM(err, log_ai_composite)

// =======================================================================
// COMPOSITE AI
// =======================================================================
std::string ai_composite::describe_self() const
{
	return "[composite_ai]";
}

ai_composite::ai_composite( default_ai_context &context, const config &cfg)
	: cfg_(cfg),stages_(),recursion_counter_(context.get_recursion_count())
{
	init_default_ai_context_proxy(context);
}

void ai_composite::on_create()
{
	LOG_AI_COMPOSITE << "side "<< get_side() << " : "<<" created AI with id=["<<
		cfg_["id"]<<"]"<<std::endl;

	// init the composite ai stages
	BOOST_FOREACH(const config &cfg_element, cfg_.child_range("stage")){
		add_stage(cfg_element);
	}

	config cfg;
	cfg["engine"] = "fai";
	engine_ptr e_ptr = get_engine_by_cfg(cfg);
	if (e_ptr) {
		e_ptr->set_ai_context(this);
	}

	boost::function2<void, std::vector<engine_ptr>&, const config&> factory_engines =
		boost::bind(&ai::ai_composite::create_engine,*this,_1,_2);

	boost::function2<void, std::vector<goal_ptr>&, const config&> factory_goals =
		boost::bind(&ai::ai_composite::create_goal,*this,_1,_2);

	boost::function2<void, std::vector<stage_ptr>&, const config&> factory_stages =
		boost::bind(&ai::ai_composite::create_stage,*this,_1,_2);

	register_vector_property("engine",get_engines(), factory_engines);
	register_vector_property("goal",get_goals(), factory_goals);
	register_vector_property("stage",stages_, factory_stages);

	register_aspect_property("aspect",get_aspects());

}


void ai_composite::create_stage(std::vector<stage_ptr> &stages, const config &cfg)
{
	engine::parse_stage_from_config(*this,cfg,std::back_inserter(stages));
}


void ai_composite::create_goal(std::vector<goal_ptr> &goals, const config &cfg)
{
	engine::parse_goal_from_config(*this,cfg,std::back_inserter(goals));
}


void ai_composite::create_engine(std::vector<engine_ptr> &engines, const config &cfg)
{
	engine::parse_engine_from_config(*this,cfg,std::back_inserter(engines));
}

ai_composite::~ai_composite()
{
}


bool ai_composite::add_stage(const config &cfg)
{
	std::vector< stage_ptr > stages;
	create_stage(stages,cfg);
	int j=0;
	BOOST_FOREACH (stage_ptr b, stages ){
		stages_.push_back(b);
		j++;
	}
	return (j>0);
}


bool ai_composite::add_goal(const config &cfg)
{
	std::vector< goal_ptr > goals;
	create_goal(goals,cfg);
	int j=0;
	BOOST_FOREACH (goal_ptr b, goals ){
		get_goals().push_back(b);
		j++;
	}
	return (j>0);
}


void ai_composite::play_turn(){
	BOOST_FOREACH(stage_ptr &s, stages_){
		s->play_stage();
	}
}


const std::string& ai_composite::get_id() const
{
	return cfg_["id"];
}



const std::string& ai_composite::get_name() const
{
	return cfg_["name"];
}


const std::string& ai_composite::get_engine() const
{
	return cfg_["engine"];
}


std::string ai_composite::evaluate(const std::string& str)
{
	config cfg;
	cfg["engine"] = "fai";//@todo 1.9 : consider allowing other engines to evaluate
	engine_ptr e_ptr = get_engine_by_cfg(cfg);
	if (!e_ptr) {
		return interface::evaluate(str);
	}
	return e_ptr->evaluate(str);
}


void ai_composite::new_turn()
{
	//@todo 1.9 replace with event system
	recalculate_move_maps();
	invalidate_defensive_position_cache();
	invalidate_keeps_cache();
	clear_additional_targets();
	unit_stats_cache().clear();
}


int ai_composite::get_recursion_count() const
{
	return recursion_counter_.get_count();
}

void ai_composite::switch_side(side_number side)
{
	set_side(side);
}

ai_context& ai_composite::get_ai_context()
{
	return *this;
}


config ai_composite::to_config() const
{
	config cfg;

	//serialize the composite ai stages
	BOOST_FOREACH(const stage_ptr &s, stages_){
		cfg.add_child("stage",s->to_config());
	}

	return cfg;
}

} //end of namespace ai
