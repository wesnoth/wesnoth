/* $Id$ */
/*
   Copyright (C) 2008 - 2010 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Contains all forward declarations for field.hpp.
 */

#ifndef GUI_DIALOGS_FIELD_FORWARD_HPP_INCLUDED
#define GUI_DIALOGS_FIELD_FORWARD_HPP_INCLUDED

namespace gui2 {

class twidget;
class twindow;

class tfield_;
class tfield_bool;
class tfield_text;

// NOTE the const must be in the template else things fail :/ bug in gcc?
template<class T, class W, class CT = const T> class tfield;
class tinteger_selector_;
typedef tfield<int, tinteger_selector_> tfield_integer;

} // namespace gui2

#endif

