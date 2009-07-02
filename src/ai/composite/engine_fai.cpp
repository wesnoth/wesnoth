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
 * FAI AI Support engine - creating specific ai components from config
 * @file ai/composite/engine_fai.cpp
 */

#include "ai.hpp"
#include "engine_fai.hpp"
#include "rca.hpp"
#include "../../foreach.hpp"
#include "../../log.hpp"

namespace ai {

namespace composite_ai {

static lg::log_domain log_ai_composite_engine_fai("ai/composite/engine/fai");
#define DBG_AI_COMPOSITE_ENGINE_FAI LOG_STREAM(debug, log_ai_composite_engine_fai)
#define LOG_AI_COMPOSITE_ENGINE_FAI LOG_STREAM(info, log_ai_composite_engine_fai)
#define ERR_AI_COMPOSITE_ENGINE_FAI LOG_STREAM(err, log_ai_composite_engine_fai)

class fai_candidate_action_wrapper : public candidate_action {
public:
	fai_candidate_action_wrapper( rca_context &context, const config &/*cfg*/, game_logic::candidate_action_ptr fai_ca, formula_ai &_formula_ai )
		: candidate_action(context,fai_ca->get_name(),fai_ca->get_type()),fai_ca_(fai_ca),formula_ai_(_formula_ai)
	{
	}


	virtual ~fai_candidate_action_wrapper() {}


	virtual double evaluate()
	{
		formula_ai_.evaluate_candidate_action(fai_ca_);
		return fai_ca_->get_score();
	}


	virtual bool execute()
	{
		return formula_ai_.execute_candidate_action(fai_ca_);
	}
private:
	game_logic::candidate_action_ptr fai_ca_;
	formula_ai &formula_ai_;
};


engine_fai::engine_fai( composite_ai_context &context, const config &cfg )
	: engine(context,cfg), formula_ai_(context)
{

}


engine_fai::~engine_fai()
{
}


void engine_fai::do_parse_candidate_action_from_config( rca_context &context, const config &cfg, std::back_insert_iterator<std::vector< candidate_action_ptr > > b ){
	game_logic::candidate_action_ptr fai_ca = formula_ai_.load_candidate_action_from_config(cfg);
	if (!fai_ca) {
		ERR_AI_COMPOSITE_ENGINE_FAI << "side "<<ai_.get_side()<< " : ERROR creating candidate_action["<<cfg["name"]<<"]"<< std::endl;
		DBG_AI_COMPOSITE_ENGINE_FAI << "config snippet contains: " << std::endl << cfg << std::endl;
		return;
	}
	candidate_action_ptr ca = candidate_action_ptr(new fai_candidate_action_wrapper(context,cfg,fai_ca,formula_ai_));
	*b = ca;

}


std::string engine_fai::get_name()
{
	return "fai";
}

} //end of namespace composite_ai

} //end of namespace ai
