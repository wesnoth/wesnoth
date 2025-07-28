/*
	Copyright (C) 2021 - 2025
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

/**
 * @file
 *
 * MacOS doesn't support std::visit when targing MacOS < 10.14 (currently we target 10.11).
 * This provides a wrapper around the STL variant API on all platforms except MacOS, which
 * instead utilizes boost::variant.
 */

#ifdef __APPLE__
#define USING_BOOST_VARIANT
#endif

#ifndef USING_BOOST_VARIANT
#include <variant>
#else
#include <boost/variant.hpp>
#endif

namespace utils
{
#ifndef USING_BOOST_VARIANT

using std::get;
using std::get_if;
using std::holds_alternative;
using std::monostate;
using std::variant;
using std::visit;

#else

using boost::get;
using boost::variant;

using monostate = boost::blank;

template<typename... Args>
inline auto visit(Args&&... args)
{
	return boost::apply_visitor(std::forward<Args>(args)...);
}

template<typename Ret, typename... Types>
inline bool holds_alternative(const boost::variant<Types...>& variant)
{
	return boost::get<Ret>(&variant) != nullptr;
}

template<typename Ret, typename... Types>
inline Ret* get_if(boost::variant<Types...>* variant)
{
	return boost::get<Ret>(variant);
}

template<typename Ret, typename... Types>
inline const Ret* get_if(const boost::variant<Types...>* variant)
{
	return boost::get<Ret>(variant);
}

#endif

template<typename... Types>
std::size_t variant_index(const variant<Types...>& var)
{
#ifndef USING_BOOST_VARIANT
	return var.index();
#else
	return var.which();
#endif
}

} // namespace utils
