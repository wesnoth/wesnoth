/*
   Copyright (C) 2008 - 2018 by Mark de Wever <koraq@xs4all.nl>
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
 * Contains all forward declarations for field.hpp.
 */

#pragma once

namespace gui2
{

class widget;
class window;

class field_base;
class field_bool;
class field_label;
class field_text;

// NOTE the const must be in the template else things fail :/ bug in gcc?
template <class T, class W, class CT = const T>
class field;
class integer_selector;
typedef field<int, integer_selector> field_integer;

} // namespace gui2
