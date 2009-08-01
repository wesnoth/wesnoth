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

static lg::log_domain log_ai_composite_engine_fai("ai/composite/engine/fai");
#define DBG_AI_COMPOSITE_ENGINE_FAI LOG_STREAM(debug, log_ai_composite_engine_fai)
#define LOG_AI_COMPOSITE_ENGINE_FAI LOG_STREAM(info, log_ai_composite_engine_fai)
#define ERR_AI_COMPOSITE_ENGINE_FAI LOG_STREAM(err, log_ai_composite_engine_fai)

#ifdef _MSC_VER
#pragma warning(push)
//silence "inherits via dominance" warnings
#pragma warning(disable:4250)
#endif

class fai_candidate_action_wrapper : public candidate_action {
public:
	fai_candidate_action_wrapper( rca_context &context, const config &cfg, game_logic::candidate_action_ptr fai_ca, formula_ai &_formula_ai )
		: candidate_action(context,cfg),fai_ca_(fai_ca),formula_ai_(_formula_ai),cfg_(cfg)//@todo 1.7: implement fai_ca->to_config()
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

	virtual config to_config() const
	{
		return cfg_;
	}
private:
	game_logic::candidate_action_ptr fai_ca_;
	formula_ai &formula_ai_;
	const config &cfg_;
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

engine_fai::engine_fai( readonly_context &context, const config &cfg )
	: engine(context,cfg)/*, formula_ai_(context,cfg.child("formula_ai"))*///@fixme
{

}


engine_fai::~engine_fai()
{
}


void engine_fai::do_parse_candidate_action_from_config( rca_context &/*context*/, const config &/*cfg*/, std::back_insert_iterator<std::vector< candidate_action_ptr > > /*b*/ ){
	/*game_logic::candidate_action_ptr fai_ca = formula_ai_.load_candidate_action_from_config(cfg);
	if (!fai_ca) {
		ERR_AI_COMPOSITE_ENGINE_FAI << "side "<<ai_.get_side()<< " : ERROR creating candidate_action["<<cfg["name"]<<"]"<< std::endl;
		DBG_AI_COMPOSITE_ENGINE_FAI << "config snippet contains: " << std::endl << cfg << std::endl;
		return;
	}
	candidate_action_ptr ca = candidate_action_ptr(new fai_candidate_action_wrapper(context,cfg,fai_ca,formula_ai_));
	*b = ca; */

}


std::string engine_fai::get_name() const
{
	return "fai";
}


config engine_fai::to_config() const
{
	config cfg = engine::to_config();
	/*cfg.add_child("formula_ai",formula_ai_.to_config());*/
	return cfg;
}

} //end of namespace ai
