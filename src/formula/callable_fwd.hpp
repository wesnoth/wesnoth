/*
   Copyright (C) 2008 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef FORMULA_CALLABLE_FWD_HPP_INCLUDED
#define FORMULA_CALLABLE_FWD_HPP_INCLUDED

#include <memory>
#include <vector>

namespace game_logic
{
class formula_callable;
class formula_debugger;

using formula_callable_ptr       = std::shared_ptr<formula_callable>;
using const_formula_callable_ptr = std::shared_ptr<const formula_callable>;
}

#endif
