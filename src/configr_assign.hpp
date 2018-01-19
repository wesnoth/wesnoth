/*
   Copyright (C) 2014 - 2018 by David White <dave@whitevine.net>
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
#include <boost/variant.hpp>

//Similar to config_of but it stores references to configs (instead of cyoping them).
struct configr_of
{
	template <typename AT>
	configr_of(const std::string& attrname, AT value) : subtags_(), data_()
	{
		this->operator()(attrname, value);
	}

	configr_of(const config& cfg) : subtags_(), data_()
	{
		this->operator()(cfg);
	}

	configr_of& operator()(const config& cfg)
	{
		data_ = &cfg;
		return *this;
	}

	configr_of& operator()(const std::string& tagname, const configr_of& child)
	{
		subtags_.emplace_back(&tagname, &child);
		return *this;
	}
	std::vector<std::pair<const std::string*, const configr_of*>> subtags_;
	const config* data_;
};
