/*
	Copyright (C) 2009 - 2024
	by Bartosz Waresiak <dragonking@o2.pl>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "formula/function.hpp"


namespace ai {
	class formula_ai;
}

namespace wfl {

class ai_function_symbol_table : public function_symbol_table {

public:
	explicit ai_function_symbol_table(ai::formula_ai& ai);
};

}
