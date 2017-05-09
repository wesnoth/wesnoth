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
 * @file
 * Stage which executes unit formulas
 * */

#pragma once

#include "ai/composite/stage.hpp"


#ifdef _MSC_VER
#pragma warning(push)
//silence "inherits via dominance" warnings
#pragma warning(disable:4250)
#endif

namespace ai {

class formula_ai;

class stage_unit_formulas: public stage {
public:
        stage_unit_formulas( ai_context &context, const config &cfg, formula_ai &fai );

        virtual ~stage_unit_formulas();

        bool do_play_stage();

        void on_create();

        config to_config() const;

private:
	formula_ai &fai_;

};

} //end of namespace ai

#ifdef _MSC_VER
#pragma warning(pop)
#endif
