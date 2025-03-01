/*
	Copyright (C) 2003 - 2025
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

#include <functional>
#include <boost/preprocessor/cat.hpp>

namespace utils {

template<typename F>
class scope_exit {
	F f_;
public:
	explicit scope_exit(F&& f) : f_(std::forward<F>(f)) {}
	~scope_exit() { f_(); }
};

struct scope_exit_syntax_support {
	template<typename F>
	scope_exit<F> operator<<(F&& f) {
		return scope_exit<F>(std::forward<F>(f));
	}
};
} // namespace utils

/**
 * Run some arbitrary code (a lambda) when the current scope exits
 * The lambda body follows this header, terminated by a semicolon.
 * @arg ... Capture clause for the lambda.
 */
#define ON_SCOPE_EXIT(...) [[maybe_unused]] const auto& BOOST_PP_CAT(scope_exit, __LINE__) = utils::scope_exit_syntax_support() << [__VA_ARGS__]()
