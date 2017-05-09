/*
   Copyright (C) 2014 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include <string>
#include "config.hpp"

class config_of
{
public:
	template <typename AT>
	config_of(const std::string& attrname, AT value)
	{
		this->operator()(attrname, value);
	}

	config_of(const std::string& tagname, const config& child)
	{
		this->operator()(tagname, child);
	}

	template <typename AT>
	config_of& operator()(const std::string& attrname, AT value)
	{
		data_[attrname] = value;
		return *this;
	}

	config_of& operator()(const std::string& tagname, const config& child)
	{
		data_.add_child(tagname, child);
		return *this;
	}

	config_of& operator()(const std::string& tagname, const config_of& child)
	{
		data_.add_child(tagname, child);
		return *this;
	}

	operator config() const
	{
		return data_;
	}
private:
	config data_;
};
