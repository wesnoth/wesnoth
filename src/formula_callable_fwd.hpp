/* $Id$ */
/*
   Copyright (C) 2008 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef FORMULA_CALLABLE_FWD_HPP_INCLUDED
#define FORMULA_CALLABLE_FWD_HPP_INCLUDED

#include <boost/intrusive_ptr.hpp>

namespace game_logic {

class formula_callable;
typedef boost::intrusive_ptr<formula_callable> formula_callable_ptr;
typedef boost::intrusive_ptr<const formula_callable> const_formula_callable_ptr;

}

#endif
