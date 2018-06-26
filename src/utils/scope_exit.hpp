/*
   Copyright (C) 2003 - 2018 the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "global.hpp"

#include <functional>

namespace utils {

class scope_exit {
	//TODO: with c++17 we could make this a template class with 'F f_' instead of 'std::function<void ()> f_';
	std::function<void ()> f_;
public:
	template<typename F>
	explicit scope_exit(F&& f) : f_(f) {}
	~scope_exit() { if(f_) { f_(); }}
};
} // namespace utils
