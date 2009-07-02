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
 * @file ai/composite/engine_fai.hpp
 */

#ifndef AI_COMPOSITE_ENGINE_FAI_HPP_INCLUDED
#define AI_COMPOSITE_ENGINE_FAI_HPP_INCLUDED

#include "../../global.hpp"

#include "engine.hpp"
#include "../contexts.hpp"
#include "../formula/ai.hpp"
#include <algorithm>

//============================================================================
namespace ai {

namespace composite_ai {

class engine_fai : public engine {
public:
	engine_fai( composite_ai_context &context, const config &cfg );

	virtual ~engine_fai();

	virtual void do_parse_candidate_action_from_config( rca_context &context, const config &cfg, std::back_insert_iterator<std::vector< candidate_action_ptr > > b );

	virtual std::string get_name();
private:
	formula_ai formula_ai_;
};

} //end of namespace composite_ai

} //end of namespace ai

#endif
