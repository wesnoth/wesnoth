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
 * Stage: fallback to other AI
 * @file ai/testing/stage_fallback.hpp
 */

#ifndef AI_TESTING_STAGE_FALLBACK_HPP_INCLUDED
#define AI_TESTING_STAGE_FALLBACK_HPP_INCLUDED

#include "../../global.hpp"

#include "../composite/stage.hpp"
#include "../interface.hpp"
#include "../../config.hpp"

#include <vector>

namespace ai {

namespace testing_ai_default {

class fallback_to_other_ai: public composite_ai::stage {
public:
	fallback_to_other_ai( composite_ai::composite_ai_context &context, const config &cfg );

	~fallback_to_other_ai();

	void do_play_stage();

	void on_create();

private:
	const config &cfg_;

	ai_ptr fallback_ai_;
};

} // end of namespace testing_ai_default

} // end of namespace ai

#endif
