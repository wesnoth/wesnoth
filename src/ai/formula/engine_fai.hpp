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

#ifndef AI_COMPOSITE_ENGINE_FAI_HPP_INCLUDED
#define AI_COMPOSITE_ENGINE_FAI_HPP_INCLUDED

#include "ai/composite/engine.hpp"
#include "ai/default/contexts.hpp"

//============================================================================
namespace ai {

class formula_ai;

class engine_fai : public engine {
public:
	engine_fai( readonly_context &context, const config &cfg );

	virtual ~engine_fai();

	virtual void do_parse_candidate_action_from_config( rca_context &context, const config &cfg, std::back_insert_iterator<std::vector< candidate_action_ptr > > b );

	virtual void do_parse_stage_from_config( ai_context &context, const config &cfg, std::back_insert_iterator<std::vector< stage_ptr > > b );

	virtual std::string evaluate(const std::string &str);

	virtual config to_config() const;

	virtual void set_ai_context(ai_context *context);
private:
	std::shared_ptr<formula_ai> formula_ai_;
};

} //end of namespace ai

#endif
