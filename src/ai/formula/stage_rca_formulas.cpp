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
 * @file ai/formula/stage_rca_formulas.cpp
 * Defines formula ai rca formulas stage
 * */


#include "stage_rca_formulas.hpp"
#include "ai.hpp"

#include "../../formula.hpp"
#include "../../formula_function.hpp"
#include "../../log.hpp"
#include <boost/lexical_cast.hpp>

static lg::log_domain log_ai("ai/stage/rca_formulas");
#define LOG_AI LOG_STREAM(info, log_ai)
#define WRN_AI LOG_STREAM(warn, log_ai)
#define ERR_AI LOG_STREAM(err, log_ai)

namespace ai {

//note: will not be documented for 1.8, as it's not finished
stage_rca_formulas::stage_rca_formulas(ai_context &context
		, const config &cfg, formula_ai &fai)
	: stage(context,cfg)
	, cfg_(cfg)
	, fai_(fai)
	, candidate_action_manager_()
{
/*	try {
		if( candidate_action_manager_.has_candidate_actions() ) {

			while( candidate_action_manager_.evaluate_candidate_actions(&fai_, get_info().units) )
			{
				game_logic::map_formula_callable callable(&fai_);
				callable.add_ref();

				candidate_action_manager_.update_callable_map( callable );

				game_logic::const_formula_ptr move_formula(candidate_action_manager_.get_best_action_formula());

				fai_.make_action(move_formula, callable);

			}
		}
	}
	catch(game_logic::formula_error& e) {
		if(e.filename == "formula") {
			e.line = 0;
		}
		fai_.handle_exception( e, "Formula error in RCA loop");
	}
*/
}


stage_rca_formulas::~stage_rca_formulas()
{
}

bool stage_rca_formulas::do_play_stage()
{
	return false;//@todo 1.9: implement
}


void stage_rca_formulas::on_create()
{
	//@todo 1.9 parse the formulas
	//candidate_action_manager_.load_config(ai_param, this, &function_table);
}


config stage_rca_formulas::to_config() const
{
	config cfg = stage::to_config();
	//@todo 1.9: serialize to config
	cfg.append(cfg_);
	return cfg;
}

} // end of namespace ai
