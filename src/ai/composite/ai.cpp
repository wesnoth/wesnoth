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
	foreach(const config &cfg_element, cfg_.child_range("stage")){
		add_stage(-1,cfg_element);
	}

	config cfg;
	cfg["engine"] = "fai";
	engine_ptr e_ptr = get_engine(cfg);
	if (e_ptr) {
		e_ptr->set_ai_context(this);
	}

}


ai_composite::~ai_composite()
{
}


bool ai_composite::add_stage(int pos, const config &cfg)
{
	if (pos<0) {
		pos = stages_.size();
	}
	std::vector< stage_ptr > stages;
	engine::parse_stage_from_config(*this,cfg,std::back_inserter(stages));
	int j=0;
	foreach (stage_ptr b, stages ){
		stages_.insert(stages_.begin()+pos+j,b);
		j++;
	}
	return (j>0);
}


bool ai_composite::add_goal(int pos, const config &cfg)
{
	if (pos<0) {
		pos = get_goals().size();
	}
	std::vector< goal_ptr > goals;
	engine::parse_goal_from_config(*this,cfg,std::back_inserter(goals));
	int j=0;
	foreach (goal_ptr b, goals ){
		get_goals().insert(get_goals().begin()+pos+j,b);
		j++;
	}
	return (j>0);
}


void ai_composite::play_turn(){
	foreach(stage_ptr &s, stages_){
		s->play_stage();
	}
}


std::string ai_composite::evaluate(const std::string& str)
{
	config cfg;
	cfg["engine"] = "fai";//@todo 1.9 : consider allowing other engines to evaluate
	engine_ptr e_ptr = get_engine(cfg);
	if (!e_ptr) {
		return interface::evaluate(str);
	}
	return e_ptr->evaluate(str);
}


void ai_composite::new_turn()
{
	//@todo 1.7 replace with event system
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
	foreach(const stage_ptr &s, stages_){
		cfg.add_child("stage",s->to_config());
	}

	return cfg;
}

component* ai_composite::get_child(const path_element &child)
{
	if (child.property=="aspect") {
		//ASPECT
		aspect_map::const_iterator a = get_aspects().find(child.id);
		if (a!=get_aspects().end()){
			return &*a->second;
		}
		return NULL;
	} else if (child.property=="stage") {
	      	//std::vector< stage_ptr >::iterator i = std::find_if(stages_.begin(),stages_.end(),path_element_matches< stage_ptr >(child));
		//if (i!=stages_.end()){
		//	return &*(*i);
		//}
		return NULL;
	} else if (child.property=="engine") {
		//ENGINE
		//@todo 1.7.5 implement
		return NULL;
	} else if (child.property=="goal") {
		//std::vector< goal_ptr >::iterator i = std::find_if(get_goals().begin(),get_goals().end(),path_element_matches< goal_ptr >(child));
		//if (i!=get_goals().end()){
		//	return &*(*i);
		//}
		return NULL;
	}

	//OOPS
	return NULL;
}


bool ai_composite::add_child(const path_element &child, const config &cfg)
{
	if (child.property=="aspect") {
		//ASPECT
		return false;//adding aspects directly is not supported - aspect['foo'].facet should be added instead
	} else if (child.property=="stage") {
	      	std::vector< stage_ptr >::iterator i = std::find_if(stages_.begin(),stages_.end(),path_element_matches< stage_ptr >(child));
		return add_stage(i-stages_.begin(),cfg);
	} else if (child.property=="engine") {
		//ENGINE
		//@todo 1.7.5 implement
		return false;
	} else if (child.property=="goal") {
		std::vector< goal_ptr >::iterator i = std::find_if(get_goals().begin(),get_goals().end(),path_element_matches< goal_ptr >(child));
		return add_goal(i-get_goals().begin(),cfg);
	}

	//OOPS
	return false;
}


bool ai_composite::change_child(const path_element &child, const config &cfg)
{
	if (child.property=="aspect") {
		//ASPECT
		return false;//changing aspects directly is not supported - aspect['foo'].facet should be changed instead
	} else if (child.property=="stage") {
	      	std::vector< stage_ptr >::iterator i = std::find_if(stages_.begin(),stages_.end(),path_element_matches< stage_ptr >(child));
		if (i!=stages_.end()) {
			//@todo 1.7.5 implement
			//return (*i)->redeploy(cfg);
		}
		return false;

	} else if (child.property=="engine") {
		//ENGINE
		//@todo 1.7.5 implement
		return false;
	} else if (child.property=="goal") {
		std::vector< goal_ptr >::iterator i = std::find_if(get_goals().begin(),get_goals().end(),path_element_matches< goal_ptr >(child));
		if (i!=get_goals().end()) {
			return (*i)->redeploy(cfg);
		}
	}

	//OOPS
	return false;
}


bool ai_composite::delete_child(const path_element &child)
{
	if (child.property=="aspect") {
		//ASPECT
		return false;//deleting aspects directly is not supported - aspect['foo'].facet should be deleted instead
	} else if (child.property=="stage") {
	      	std::vector< stage_ptr >::iterator i = std::find_if(stages_.begin(),stages_.end(),path_element_matches< stage_ptr >(child));
		if (i!=stages_.end()) {
			stages_.erase(i);
			return true;
		}
		return false;
	} else if (child.property=="engine") {
		//ENGINE
		//@todo 1.7.5 implement
		return false;
	} else if (child.property=="goal") {
		std::vector< goal_ptr >::iterator i = std::find_if(get_goals().begin(),get_goals().end(),path_element_matches< goal_ptr >(child));
		if (i!=get_goals().end()) {
			get_goals().erase(i);
			return true;
		}
		return false;
	}

	//OOPS
	return false;
}



} //end of namespace ai
