/*
   Copyright (C) 2009 - 2016 by Yurii Chernyi <terraninfo@terraninfo.net>
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
 * Strategic recruitment routine, for experimentation
 */

#ifndef AI_TESTING_CA_TESTING_RECRUITMENT_HPP_INCLUDED
#define AI_TESTING_CA_TESTING_RECRUITMENT_HPP_INCLUDED

#include "../composite/rca.hpp"

#include "../../unit_map.hpp"

#ifdef _MSC_VER
#pragma warning(push)
//silence "inherits via dominance" warnings
#pragma warning(disable:4250)
#endif

namespace pathfind {

struct plain_route;

} //of namespace pathfind

namespace ai {

namespace testing_ai_default {

class testing_recruitment_phase : public candidate_action {
public:

	testing_recruitment_phase( rca_context &context, const config &cfg );

	virtual ~testing_recruitment_phase();

	virtual double evaluate();

	virtual void execute();

protected:
	void do_recruit(int max_units_to_recruit, double quality_factor);
};

} // of namespace testing_ai_default

} // of namespace ai

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif

