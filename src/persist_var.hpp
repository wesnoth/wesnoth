/*
   Copyright (C) 2010 - 2017 by Jody Northup
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef PERSIST_VAR_H_INCLUDED
#define PERSIST_VAR_H_INCLUDED

class vconfig;

void verify_and_set_global_variable(const vconfig &pcfg);
void verify_and_get_global_variable(const vconfig &pcfg);
void verify_and_clear_global_variable(const vconfig &pcfg);
#endif
