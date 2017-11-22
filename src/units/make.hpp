/*
   Copyright (C) 2014 - 2017 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "units/gender.hpp"
#include "units/ptr.hpp"

class config;
class unit_type;
class vconfig;

unit_ptr make_unit_ptr(const config& cfg, bool use_traits = false, const vconfig* vcfg = nullptr);
unit_ptr make_unit_ptr(const unit_type& t, int side, bool real_unit, unit_gender gender = unit_gender::NUM_GENDERS);
unit_ptr make_unit_ptr(const unit& u);
