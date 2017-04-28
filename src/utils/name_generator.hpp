/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef NAME_GENERATOR_HPP_INCLUDED
#define NAME_GENERATOR_HPP_INCLUDED

#include "global.hpp"

#include <string>
#include <map>
#include <exception>

class name_generator_invalid_exception : public std::exception {
public:
	name_generator_invalid_exception(const char* errMessage):errMessage_(errMessage) {}
	const char* what() const NOEXCEPT { return errMessage_; }

private:
	const char* errMessage_;
};


class name_generator {
public:
	// Defined in name_generator_factory.cpp
	std::string generate(const std::map<std::string,std::string>& variables) const;
	virtual std::string generate() const { return ""; }
	name_generator() {}
	virtual ~name_generator() {}
};

class proxy_name_generator : public name_generator {
	const name_generator& base;
public:
	proxy_name_generator(const name_generator& b) : base(b) {}
	std::string generate() const override { return base.generate(); }
};

#endif
