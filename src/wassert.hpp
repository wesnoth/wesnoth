/*
   Copyright (C) 2005 by Isaac Clerencia <isaac@sindominio.net>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef WASSERT_HPP_INCLUDED
#define WASSERT_HPP_INCLUDED

#ifdef _MSC_VER
#	define STANDARD_ASSERT_DOES_NOT_WORK
#endif

#ifdef STANDARD_ASSERT_DOES_NOT_WORK
void wassert(bool expression);
#else
#include <cassert>
#define wassert assert
#endif

#endif
