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
 * @file ai/formula/stage_rca_formulas.hpp
 * Stage which executes rca formulas
 * */


#ifndef AI_FORMULA_STAGE_RCA_FORMULAS_HPP_INCLUDED
#define AI_FORMULA_STAGE_RCA_FORMULAS_HPP_INCLUDED

#include "../composite/stage.hpp"
#include "candidates.hpp"

#ifdef _MSC_VER
#pragma warning(push)
//silence "inherits via dominance" warnings
#pragma warning(disable:4250)
#endif


namespace ai {

class formula_ai;

class stage_rca_formulas: public stage {
public:
        stage_rca_formulas( ai_context &context, const config &cfg, formula_ai &fai );

        virtual ~stage_rca_formulas();

        bool do_play_stage();

        void on_create();

        config to_config() const;

private:
        const config &cfg_;
	formula_ai &fai_;
	game_logic::candidate_action_manager candidate_action_manager_;

};

} //end of namespace ai

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
