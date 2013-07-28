/*
	Copyright (C) 2013 by Pierre Talbot <ptalbot@mopong.net>
	Part of the Battle for Wesnoth Project http://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#ifndef UMCD_BOOST_THREAD_WORKAROUND_HPP
#define UMCD_BOOST_THREAD_WORKAROUND_HPP

// boost::thread < 1.51 conflicts with C++11-capable compilers
#if BOOST_VERSION < 105100
	#include <ctime>
	#undef TIME_UTC
#endif

#endif // UMCD_BOOST_THREAD_WORKAROUND_HPP