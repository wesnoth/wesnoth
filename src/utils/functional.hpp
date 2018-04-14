/*
 Copyright (C) 2003 - 2018 by the Battle for Wesnoth Project http://www.wesnoth.org/

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY.

 See the COPYING file for more details.
 */

/*
 * Wrapper header to allow use of placeholder variables without a namespace.
 */

#pragma once

#include <functional>
#include <boost/bind.hpp>

// We'd like to just say "using namespace std::placeholders", but unfortunately
// that clashes with Boost.Bind's placeholders in some cases (even if bind.hpp is not included).
// Instead, we specialize std::is_placeholder for the Boost placeholders,
// so that Boost placeholders can be passed to std::bind.

namespace std { // Some compilers can't handle it being specialized in the global scope
	template<int N>
	struct is_placeholder<boost::arg<N>> : public integral_constant<int, N> {};
}
