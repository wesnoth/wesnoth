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
 * @file ai/composite/engine_default.hpp
 */

#ifndef AI_COMPOSITE_ENGINE_DEFAULT_HPP_INCLUDED
#define AI_COMPOSITE_ENGINE_DEFAULT_HPP_INCLUDED

#include "../../global.hpp"

#include "engine.hpp"
#include "../contexts.hpp"
#include <algorithm>

//============================================================================
namespace ai {

namespace composite_ai {

class engine_cpp : public engine {
public:
	engine_cpp( composite_ai_context &context, const config &cfg );

	virtual ~engine_cpp();

	virtual void do_parse_candidate_action_from_config( rca_context &context, const config &cfg, std::back_insert_iterator<std::vector< candidate_action_ptr > > b );

	virtual void do_parse_stage_from_config( const config &cfg, std::back_insert_iterator<std::vector< stage_ptr > > b );


	virtual std::string get_name();
};

} //end of namespace composite_ai

} //end of namespace ai

#endif
