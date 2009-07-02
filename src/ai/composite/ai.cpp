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
 * Composite AI with turn sequence which is a vector of stages
 * @file ai/composite/ai.cpp
 */

#include "ai.hpp"
#include "stage.hpp"
#include "../manager.hpp"
#include "../../foreach.hpp"
#include "../../log.hpp"

namespace ai {

namespace composite_ai {

static lg::log_domain log_ai_composite("ai/composite");
#define DBG_AI_COMPOSITE LOG_STREAM(debug, log_ai_composite)
#define LOG_AI_COMPOSITE LOG_STREAM(info, log_ai_composite)
#define ERR_AI_COMPOSITE LOG_STREAM(err, log_ai_composite)

// =======================================================================
// COMPOSITE AI
// =======================================================================
std::string ai_composite::describe_self(){
	return "[composite_ai]";
}

ai_composite::ai_composite( default_ai_context &context)
	: recursion_counter_(context.get_recursion_count())
{
	init_default_ai_context_proxy(context);
}

void ai_composite::on_create()
{
	const config& ai_global_parameters = ai::manager::get_active_ai_global_parameters_for_side(get_side());
	LOG_AI_COMPOSITE << "side "<< get_side() << " : "<<" created AI with id=["<<
		ai_global_parameters["id"]<<"]"<<std::endl;

	//init the composite ai engines
	foreach(const config &cfg_element, ai_global_parameters.child_range("engine")){
		engine::parse_engine_from_config(*this,cfg_element,std::back_inserter(engines_));
	}

	// init the composite ai stages
	foreach(const config &cfg_element, ai_global_parameters.child_range("stage")){
		engine::parse_stage_from_config(*this,cfg_element,std::back_inserter(stages_));
	}

}

ai_composite::~ai_composite()
{
}


void ai_composite::play_turn(){
	foreach(stage_ptr &s, stages_){
		s->play_stage();
	}
}


void ai_composite::new_turn()
{
	//todo 1.7 replace with event system
	recalculate_move_maps();
	invalidate_attack_depth_cache();
	invalidate_avoided_locations_cache();
	invalidate_defensive_position_cache();
	invalidate_recent_attacks_list();
	invalidate_keeps_cache();
	unit_stats_cache().clear();
}


engine_ptr ai_composite::get_engine(const config& cfg)
{
	const std::string& engine_name = cfg["engine"];
	std::vector<engine_ptr>::iterator en = engines_.begin();
	while (en!=engines_.end() && ((*en)->get_name()!=engine_name)) {
		en++;
	}

	if (en != engines_.end()){
		return *en;
	}

	engine_factory::factory_map::iterator eng = engine_factory::get_list().find(engine_name);
	if (eng == engine_factory::get_list().end()){
		ERR_AI_COMPOSITE << "side "<<get_side()<<" : UNABLE TO FIND engine["<< 
		engine_name <<"]" << std::endl;
		DBG_AI_COMPOSITE << "config snippet contains: " << std::endl << cfg << std::endl;
		return engine_ptr();
	}

	engine_ptr new_engine = eng->second->get_new_instance(*this,engine_name);
	if (!new_engine) {
		ERR_AI_COMPOSITE << "side "<<get_side()<<" : UNABLE TO CREATE engine["<< 
		engine_name <<"] " << std::endl;
		DBG_AI_COMPOSITE << "config snippet contains: " << std::endl << cfg << std::endl;
		return engine_ptr();
	}
	engines_.push_back(new_engine);
	return engines_.back();
}

int ai_composite::get_recursion_count() const
{
	return recursion_counter_.get_count();
}

void ai_composite::switch_side(side_number side)
{
	set_side(side);
}

composite_ai_context& ai_composite::get_composite_ai_context()
{
	return *this;
}


} //end of namespace composite_ai

} //end of namespace ai
