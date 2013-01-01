/*
   Copyright (C) 2009 - 2013 by Bartosz Waresiak <dragonking@o2.pl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef FORMULA_AI_FUNCTION_TABLE_HPP_INCLUDED
#define FORMULA_AI_FUNCTION_TABLE_HPP_INCLUDED

#include "formula_function.hpp"

#include <set>

namespace ai {
	class formula_ai;
}

namespace game_logic {

class ai_function_symbol_table : public function_symbol_table {

public:
	explicit ai_function_symbol_table(ai::formula_ai& ai) :
		ai_(ai),
		move_functions()
	{}

	expression_ptr create_function(const std::string& fn,
	                               const std::vector<expression_ptr>& args) const;

private:
	ai::formula_ai& ai_;
	std::set<std::string> move_functions;
};

}

#endif	/* FORMULA_AI_FUNCTION_TABLE_HPP_INCLUDED */

