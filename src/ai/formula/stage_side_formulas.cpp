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
 * @file
 * Defines formula ai side formulas stage
 */


#include "ai/formula/stage_side_formulas.hpp"
#include "ai/formula/ai.hpp"

#include "formula/formula.hpp"
#include "formula/function.hpp"
#include "log.hpp"

static lg::log_domain log_ai("ai/stage/side_formulas");
#define LOG_AI LOG_STREAM(info, log_ai)
#define WRN_AI LOG_STREAM(warn, log_ai)
#define ERR_AI LOG_STREAM(err, log_ai)

namespace ai {

stage_side_formulas::stage_side_formulas(ai_context &context, const config &cfg, formula_ai &fai)
       	: stage(context,cfg), cfg_(cfg), fai_(fai), move_formula_()
{

}


stage_side_formulas::~stage_side_formulas()
{
}

bool stage_side_formulas::do_play_stage()
{
	game_logic::map_formula_callable callable(&fai_);
	try {
		if (move_formula_) {
			while( !fai_.make_action(move_formula_,callable).is_empty() ) { }
		} else {
			WRN_AI << "Side formula skipped, maybe it's empty or incorrect" << std::endl;
		}
	}
	catch(game_logic::formula_error& e) {
		if(e.filename == "formula") {
			e.line = 0;
		}
		fai_.handle_exception( e, "Formula error");
	}
	return false;
}


void stage_side_formulas::on_create()
{
	move_formula_ = fai_.create_optional_formula(cfg_["move"]);
}


config stage_side_formulas::to_config() const
{
	config cfg = stage::to_config();
	///@todo 1.7: serialize to config
	cfg.append(cfg_);
	return cfg;
}

} // end of namespace ai
