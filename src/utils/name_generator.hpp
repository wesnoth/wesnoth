/*
   Copyright (C) 2003 - 2016 by David White <dave@whitevine.net>
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

#include <string>

class name_generator {
public:
	virtual std::string generate() const = 0;
	virtual bool is_valid() const {return true;}
	virtual ~name_generator() {}
};

class proxy_name_generator : public name_generator {
	const name_generator& base;
public:
	proxy_name_generator(const name_generator& b) : base(b) {}
	std::string generate() const override {return base.generate();}
	bool is_valid() const override {return base.is_valid();}
};

#endif
