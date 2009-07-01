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

engine_fai::engine_fai( composite_ai_context &context, const config &cfg )
	: engine(context,cfg)
{
}


engine_fai::~engine_fai()
{
}


void engine_fai::do_parse_candidate_action_from_config( rca_context &context, const config &cfg, std::back_insert_iterator<std::vector< candidate_action_ptr > > b ){
	//*b = new_candidate_action;

}


std::string engine_fai::get_name()
{
	return "fai";
}

} //end of namespace composite_ai

} //end of namespace ai
