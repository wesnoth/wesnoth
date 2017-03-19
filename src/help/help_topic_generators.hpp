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

#ifndef HELP_TOPIC_GENERATORS_HPP
#define HELP_TOPIC_GENERATORS_HPP

#include "help_impl.hpp"

#include <string>                       // for string
#include <utility>                      // for pair
#include <vector>                       // for vector
class terrain_type;  // lines 20-20
class unit_type;

namespace help {

class terrain_topic_generator: public topic_generator
{
	const terrain_type& type_;


public:
	terrain_topic_generator(const terrain_type& type) : type_(type) {}

	virtual std::string operator()() const;
};



class unit_topic_generator: public topic_generator
{
	const unit_type& type_;
	const std::string variation_;
	typedef std::pair< std::string, unsigned > item;
	void push_header(std::vector< item > &row,  const std::string& name) const;
public:
	unit_topic_generator(const unit_type &t, std::string variation="") : type_(t), variation_(variation) {}
	virtual std::string operator()() const;
};

} // end namespace help/

#endif
