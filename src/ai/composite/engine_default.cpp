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
 * CPP AI Support engine - creating specific ai components from config
 * @file ai/composite/engine_default.cpp
 */

#include "ai.hpp"
#include "engine_default.hpp"
#include "rca.hpp"
#include "../../foreach.hpp"
#include "../../log.hpp"

namespace ai {

namespace composite_ai {

static lg::log_domain log_ai_composite_engine_cpp("ai/composite/engine/cpp");
#define DBG_AI_COMPOSITE_ENGINE_CPP LOG_STREAM(debug, log_ai_composite_engine_cpp)
#define LOG_AI_COMPOSITE_ENGINE_CPP LOG_STREAM(info, log_ai_composite_engine_cpp)
#define ERR_AI_COMPOSITE_ENGINE_CPP LOG_STREAM(err, log_ai_composite_engine_cpp)

engine_cpp::engine_cpp( composite_ai_context &context, const config &cfg )
	: engine(context,cfg)
{
}

engine_cpp::~engine_cpp()
{
}


void engine_cpp::do_parse_candidate_action_from_config( rca_context &context, const config &cfg, std::back_insert_iterator<std::vector< candidate_action_ptr > > b ){
	candidate_action_factory::factory_map::iterator f = candidate_action_factory::get_list().find(cfg["name"]);
	if (f == candidate_action_factory::get_list().end()){
		ERR_AI_COMPOSITE_ENGINE_CPP << "side "<<ai_.get_side()<< " : UNKNOWN candidate_action["<<cfg["name"]<<"]"<< std::endl;
		DBG_AI_COMPOSITE_ENGINE_CPP << "config snippet contains: " << std::endl << cfg << std::endl;
		return;
	}
	candidate_action_ptr new_candidate_action = f->second->get_new_instance(context,cfg);
	if (!new_candidate_action) {
		ERR_AI_COMPOSITE_ENGINE_CPP << "side "<<ai_.get_side()<< " : UNABLE TO CREATE candidate_action["<<cfg["name"]<<"]"<< std::endl;
		DBG_AI_COMPOSITE_ENGINE_CPP << "config snippet contains: " << std::endl << cfg << std::endl;
		return;
	}
	*b = new_candidate_action;

}

void engine_cpp::do_parse_stage_from_config( const config &cfg, std::back_insert_iterator<std::vector< stage_ptr > > b )
{
	stage_factory::factory_map::iterator f = stage_factory::get_list().find(cfg["name"]);
	if (f == stage_factory::get_list().end()){
		ERR_AI_COMPOSITE_ENGINE_CPP << "side "<<ai_.get_side()<< " : UNKNOWN stage["<<cfg["name"]<<"]"<< std::endl;
		DBG_AI_COMPOSITE_ENGINE_CPP << "config snippet contains: " << std::endl << cfg << std::endl;
		return;
	}
	stage_ptr new_stage = f->second->get_new_instance(ai_,cfg);
	if (!new_stage) {
		ERR_AI_COMPOSITE_ENGINE_CPP << "side "<<ai_.get_side()<< " : UNABLE TO CREATE stage["<<cfg["name"]<<"]"<< std::endl;
		DBG_AI_COMPOSITE_ENGINE_CPP << "config snippet contains: " << std::endl << cfg << std::endl;
		return;
	}
	*b = new_stage;
}


std::string engine_cpp::get_name()
{
	return "cpp";
}

} //end of namespace composite_ai

} //end of namespace ai
