/* $Id$ */
/*
   copyright (c) 2008 - 2009 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/

/**
 * @file field-fwd.hpp
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

