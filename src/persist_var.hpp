/* $Id$ */
/*
   Copyright (C) 2003 - 2010 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef PERSIST_VAR_H_INCLUDED
#define PERSIST_VAR_H_INCLUDED
void verify_and_set_global_variable(const vconfig &pcfg);
void verify_and_get_global_variable(const vconfig &pcfg);
void verify_and_clear_global_variable(const vconfig &pcfg);
#endif
