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
 * FAI AI Support engine - creating specific ai components from config
 * @file
 */

#include "ai/formula/ai.hpp"
#include "ai/formula/engine_fai.hpp"
#include "ai/composite/rca.hpp"
#include "ai/formula/ai.hpp"
#include "ai/formula/candidates.hpp"
#include "ai/formula/stage_side_formulas.hpp"
#include "ai/formula/stage_unit_formulas.hpp"
#include "log.hpp"
#include "units/unit.hpp"

namespace ai {

static lg::log_domain log_ai_engine_fai("ai/engine/fai");
#define DBG_AI_ENGINE_FAI LOG_STREAM(debug, log_ai_engine_fai)
#define LOG_AI_ENGINE_FAI LOG_STREAM(info, log_ai_engine_fai)
#define ERR_AI_ENGINE_FAI LOG_STREAM(err, log_ai_engine_fai)

#ifdef _MSC_VER
#pragma warning(push)
//silence "inherits via dominance" warnings
#pragma warning(disable:4250)
#endif

class fai_candidate_action_wrapper : public candidate_action {
public:
	fai_candidate_action_wrapper( rca_context &context, const config &cfg, game_logic::candidate_action_ptr fai_ca, formula_ai &_formula_ai )
		: candidate_action(context,cfg),fai_ca_(fai_ca),formula_ai_(_formula_ai),cfg_(cfg)///@todo 1.7: implement fai_ca->to_config()
	{

}

	virtual ~fai_candidate_action_wrapper() {}


	virtual double evaluate()
	{
		formula_ai_.evaluate_candidate_action(fai_ca_);
		return fai_ca_->get_score();
	}


	virtual void execute()
	{
		formula_ai_.execute_candidate_action(fai_ca_);
	}

	virtual config to_config() const
	{
		return cfg_;
	}
private:
	game_logic::candidate_action_ptr fai_ca_;
	formula_ai &formula_ai_;
	const config cfg_;
};

engine_fai::engine_fai( readonly_context &context, const config &cfg )
	: engine(context,cfg), formula_ai_(new formula_ai(context,cfg.child_or_empty("formula_ai")))
{
	name_ = "fai";
	formula_ai_->on_create();
}


engine_fai::~engine_fai()
{
}


void engine_fai::do_parse_candidate_action_from_config( rca_context &context, const config &cfg, std::back_insert_iterator<std::vector< candidate_action_ptr > > b ){
	game_logic::candidate_action_ptr fai_ca = formula_ai_->load_candidate_action_from_config(cfg);
	if (!fai_ca) {
		ERR_AI_ENGINE_FAI << "side "<<ai_.get_side()<< " : ERROR creating candidate_action["<<cfg["name"]<<"]"<< std::endl;
		DBG_AI_ENGINE_FAI << "config snippet contains: " << std::endl << cfg << std::endl;
		return;
	}
	candidate_action_ptr ca = candidate_action_ptr(new fai_candidate_action_wrapper(context,cfg,fai_ca,*formula_ai_));
	*b = ca;

}

void engine_fai::do_parse_stage_from_config( ai_context &context, const config &cfg, std::back_insert_iterator<std::vector< stage_ptr > > b )
{
	if (!cfg) {
		return;
	}
	const std::string &name = cfg["name"];
	stage_ptr st_ptr;

	//dropped from 1.8, as it's not ready
	//if (name=="rca_formulas") {
	//	st_ptr = stage_ptr(new stage_rca_formulas(context,cfg,formula_ai_));

	if (name=="side_formulas") {
		st_ptr = stage_ptr(new stage_side_formulas(context,cfg,*formula_ai_));
	} else if (name=="unit_formulas") {
		st_ptr = stage_ptr(new stage_unit_formulas(context,cfg,*formula_ai_));
	} else {
		ERR_AI_ENGINE_FAI << "unknown type of formula_ai stage: ["<< name <<"]"<<std::endl;
	}
	if (st_ptr) {
		st_ptr->on_create();
		*b = st_ptr;
	}
}

std::string engine_fai::evaluate(const std::string &str)
{
	return formula_ai_->evaluate(str);
}


void engine_fai::set_ai_context(ai_context *context)
{
	if (context!=nullptr) {
		DBG_AI_ENGINE_FAI << "fai engine: ai_context is set" << std::endl;
	} else {
		DBG_AI_ENGINE_FAI << "fai engine: ai_context is cleared" << std::endl;
	}
	formula_ai_->set_ai_context(context);
}


config engine_fai::to_config() const
{
	config cfg = engine::to_config();
	cfg.add_child("formula_ai",formula_ai_->to_config());
	return cfg;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

} //end of namespace ai
