/* $Id$ */
/*
   Copyright (C) 2004 by Philippe Plantier <ayin@anathas.org>
   Copyright (C) 2005 - 2011 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 *  @file
 *  t_token provides a repository of single word references to unqiue constant strings
 *  allowing for fast comparison operators.
 */

#include "token.hpp"

namespace n_token {

/// Do not inline this enforces static initialization order and static de-initialization order
const t_token & t_token::z_empty() {
	//This is NOTa memory leak
	//It is static so it is only allocated once and not de-allocated until the program terminates.
	///If changed to a static reference it may cause a static de-initialization 
	//core dump when the destructor for z_empty is called here before its last use in another file.
	//Do not remove the = new t_token("", false);
	static t_token *z_empty = new t_token("", false);
	return *z_empty;
}
}//end namepace


