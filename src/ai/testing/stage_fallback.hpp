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
 * Stage: fallback to other AI
 * @file
 */

#ifndef AI_TESTING_STAGE_FALLBACK_HPP_INCLUDED
#define AI_TESTING_STAGE_FALLBACK_HPP_INCLUDED

#include "ai/composite/stage.hpp"
#include "ai/interface.hpp"

#ifdef _MSC_VER
#pragma warning(push)
//silence "inherits via dominance" warnings
#pragma warning(disable:4250)
#endif

namespace ai {

namespace testing_ai_default {

class fallback_to_other_ai: public stage {
public:
	fallback_to_other_ai( ai_context &context, const config &cfg );

	~fallback_to_other_ai();

	bool do_play_stage();

	void on_create();

	config to_config() const;

private:
	const config &cfg_;

	ai_ptr fallback_ai_;
};

} // end of namespace testing_ai_default

} // end of namespace ai

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
