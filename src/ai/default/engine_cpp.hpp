/*
   Copyright (C) 2009 - 2018 by Yurii Chernyi <terraninfo@terraninfo.net>
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
 * CPP AI Support engine - creating specific ai components from config
 * @file
 */

#pragma once

#include "ai/composite/engine.hpp"

//============================================================================
namespace ai {

class engine_cpp : public engine {
public:
	engine_cpp( readonly_context &context, const config &cfg );


	virtual ~engine_cpp();


	void do_parse_aspect_from_config( const config &cfg, const std::string &id, std::back_insert_iterator<std::vector< aspect_ptr >> b );


	virtual void do_parse_candidate_action_from_config( rca_context &context, const config &cfg, std::back_insert_iterator<std::vector< candidate_action_ptr >> b );


	virtual void do_parse_stage_from_config( ai_context &context, const config &cfg, std::back_insert_iterator<std::vector< stage_ptr >> b );


	virtual void do_parse_goal_from_config(const config &cfg, std::back_insert_iterator<std::vector< goal_ptr >> b );

	virtual void do_parse_engine_from_config(const config &cfg, std::back_insert_iterator<std::vector< engine_ptr >> b );

};

} //end of namespace ai
