/* $Id$ */
/*
   Copyright (C) 2008 by David White <dave@whitevine.net>
                 2008 by Richard Kettering <kettering.richard@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/


#ifndef ASSERTS_HPP_INCLUDED
#define ASSERTS_HPP_INCLUDED

#include <iostream>

//various asserts of standard "equality" tests, such as "equals", "not equals", "greater than", etc.  Example usage:
//assert_ne(x, y);
#define ASSERT_EQ(a,b) if((a) != (b)) { std::cerr << __FILE__ << ":" << __LINE__ << " ASSERT EQ FAILED: " << #a << " != " << #b << ": " << (a) << " != " << (b) << "\n"; abort(); }

#define ASSERT_NE(a,b) if((a) == (b)) { std::cerr << __FILE__ << ":" << __LINE__ << " ASSERT NE FAILED: " << #a << " == " << #b << ": " << (a) << " == " << (b) << "\n"; abort(); }

#define ASSERT_GE(a,b) if((a) < (b)) { std::cerr << __FILE__ << ":" << __LINE__ << " ASSERT GE FAILED: " << #a << " == " << #b << ": " << (a) << " == " << (b) << "\n"; abort(); }

#define ASSERT_LE(a,b) if((a) > (b)) { std::cerr << __FILE__ << ":" << __LINE__ << " ASSERT LE FAILED: " << #a << " == " << #b << ": " << (a) << " == " << (b) << "\n"; abort(); }

#define ASSERT_GT(a,b) if((a) <= (b)) { std::cerr << __FILE__ << ":" << __LINE__ << " ASSERT GT FAILED: " << #a << " == " << #b << ": " << (a) << " == " << (b) << "\n"; abort(); }

#define assert_lt(a,b) if((a) >= (b)) { std::cerr << __FILE__ << ":" << __LINE__ << " ASSERT LT FAILED: " << #a << " == " << #b << ": " << (a) << " == " << (b) << "\n"; abort(); }

//for custom logging.  Example usage:
//assert_log(x != y, "x not equal to y. Value of x: " << x << ", y: " << y);
#define ASSERT_LOG(a,b) if( !(a) ) { std::cerr << __FILE__ << ":" << __LINE__ << " ASSSERTION FAILED: " << (b) << "\n"; abort(); }


#endif

