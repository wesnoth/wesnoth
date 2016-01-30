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
 * Strategic movement routine, for experimentation
 */

#ifndef AI_TESTING_CA_TESTING_GLOBAL_FALLBACK_HPP_INCLUDED
#define AI_TESTING_CA_TESTING_GLOBAL_FALLBACK_HPP_INCLUDED

#include "../composite/rca.hpp"

#include "../../unit_map.hpp"

#ifdef _MSC_VER
#pragma warning(push)
//silence "inherits via dominance" warnings
#pragma warning(disable:4250)
#endif

namespace ai {

namespace testing_ai_default {

class global_fallback_phase : public candidate_action {
public:

	global_fallback_phase( rca_context &context, const config &cfg );

	virtual ~global_fallback_phase();

	virtual double evaluate();

	virtual void execute();

protected:
};

} // of namespace testing_ai_default

} // of namespace ai

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
