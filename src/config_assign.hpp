/*
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef CONFIG_ASSIGN_H_INCLUDED
#define CONFIG_ASSIGN_H_INCLUDED

#include <string>
#include "config.hpp"

class config_of
{
public:
	config_of(const std::string& attrname, int value);
	config_of(const std::string& attrname, unsigned int value);
	config_of(const std::string& attrname, bool value);
	config_of(const std::string& attrname, double value);
	config_of(const std::string& attrname, const std::string& value);
	config_of(const std::string& attrname, config::attribute_value value);
	config_of(const std::string& tagname, const config& child);
	
	config_of& operator()(const std::string& attrname, int value);
	config_of& operator()(const std::string& attrname, unsigned int value);
	config_of& operator()(const std::string& attrname, bool value);
	config_of& operator()(const std::string& attrname, double value);
	config_of& operator()(const std::string& attrname, const std::string& value);
	config_of& operator()(const std::string& attrname, config::attribute_value value);
	config_of& operator()(const std::string& tagname, const config& child);
	operator config() const;
private:
	config data_;
};
#endif