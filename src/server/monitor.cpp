/* $Id$ */
/*
   Copyright (C) 2008 by Pauli Nieminen <paniemin@cc.hut.fi>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! Here is the implementation for server monitoring thread
//! Initial target is to have it useful in posix systems
//! and later add support for windows if possible.
//! If a system doesn't support monitoring it is not compiled.

#include "monitor.hpp"

#ifdef SERVER_MONITOR
#else // SERVER_MONITOR
// We need to define monitor hooks anyway as NOP so code will compile
#endif // SERVER_MONITOR

