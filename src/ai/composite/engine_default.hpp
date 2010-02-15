/* $Id$ */
/*
   Copyright (C) 2009 - 2010 by Yurii Chernyi <terraninfo@terraninfo.net>
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

class engine_cpp : public engine {
public:
	engine_cpp( readonly_context &context, const config &cfg );


	virtual ~engine_cpp();


	void do_parse_aspect_from_config( const config &cfg, const std::string &id, std::back_insert_iterator<std::vector< aspect_ptr > > b );


	virtual void do_parse_candidate_action_from_config( rca_context &context, const config &cfg, std::back_insert_iterator<std::vector< candidate_action_ptr > > b );


	virtual void do_parse_stage_from_config( ai_context &context, const config &cfg, std::back_insert_iterator<std::vector< stage_ptr > > b );


	virtual void do_parse_goal_from_config(const config &cfg, std::back_insert_iterator<std::vector< goal_ptr > > b );

	virtual void do_parse_engine_from_config(const config &cfg, std::back_insert_iterator<std::vector< engine_ptr > > b );

};

} //end of namespace ai

#endif
