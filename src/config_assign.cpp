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
#include "config_assign.hpp"
#include "config.hpp"


config_of::config_of(const std::string& attrname, bool value)
{
	this->operator()(attrname, value);
}

config_of::config_of(const std::string& attrname, int value)
{
	this->operator()(attrname, value);
}

config_of::config_of(const std::string& attrname, unsigned int value)
{
	this->operator()(attrname, value);
}

config_of::config_of(const std::string& attrname, double value)
{
	this->operator()(attrname, value);
}

config_of::config_of(const std::string& attrname, const std::string& value)
{
	this->operator()(attrname, value);
}
config_of::config_of(const std::string& attrname, config::attribute_value value)
{
	this->operator()(attrname, value);
}

config_of::config_of(const std::string& tagname, const config& child)
{
	this->operator()(tagname, child);
}

config_of& config_of::operator()(const std::string& attrname, bool value)
{
	data_[attrname] = value;
	return *this;
}
config_of& config_of::operator()(const std::string& attrname, int value)
{
	data_[attrname] = value;
	return *this;
}

config_of& config_of::operator()(const std::string& attrname, unsigned  int value)
{
	data_[attrname] = value;
	return *this;
}
config_of& config_of::operator()(const std::string& attrname, double value)
{
	data_[attrname] = value;
	return *this;
}
config_of& config_of::operator()(const std::string& attrname, const std::string& value)
{
	data_[attrname] = value;
	return *this;
}
config_of& config_of::operator()(const std::string& attrname, config::attribute_value value)
{
	data_[attrname] = value;
	return *this;
}

config_of& config_of::operator()(const std::string& tagname, const config& child)
{
	data_.add_child(tagname, child);
	return *this;
}

config_of::operator config() const
{
	return data_;
}
