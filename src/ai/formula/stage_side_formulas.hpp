/*
   Copyright (C) 2009 - 2018 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
 * Stage which executes side formulas
 * */

#pragma once

#include "ai/composite/stage.hpp"
#include "formula/formula_fwd.hpp"

namespace ai {

class formula_ai;

class stage_side_formulas: public stage {
public:
        stage_side_formulas( ai_context &context, const config &cfg, formula_ai &fai );

        virtual ~stage_side_formulas();

        bool do_play_stage();

        void on_create();

        config to_config() const;

private:
        const config &cfg_;
	formula_ai &fai_;
	wfl::const_formula_ptr move_formula_;

};

} //end of namespace ai
