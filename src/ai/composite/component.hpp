/* $Id$ */
/*
   Copyright (C) 2009 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file ai/composite/component.hpp
 * A component of the AI framework
 */

#ifndef AI_COMPOSITE_COMPONENT_HPP_INCLUDED
#define AI_COMPOSITE_COMPONENT_HPP_INCLUDED

#include "../../global.hpp"

#include "../game_info.hpp"

#include <vector>

#ifdef _MSC_VER
#pragma warning(push)
//silence "inherits via dominance" warnings
#pragma warning(disable:4250)
#endif

//============================================================================
namespace ai {


struct path_element {
	path_element()
		: property()
		, id()
		, position(0)
	{
	}

	std::string property;
	std::string id;
	int position;
};


class component {
public:
	component() {};
	virtual ~component() {};
	virtual component* get_child(const path_element &child);
	virtual bool change_child(const path_element &child, const config &cfg);
	virtual bool add_child(const path_element &child, const config &cfg);
	virtual bool delete_child(const path_element &child);
};

class component_manager {
public:
	static bool add_component(component *root, const std::string &path, const config &cfg);
	static bool change_component(component *root, const std::string &path, const config &cfg);
	static bool delete_component(component *root, const std::string &path);
};



template<typename T>
class path_element_matches{
public:
	path_element_matches(const path_element &element)
		: count_(0), element_(element)
	{
	}
	virtual ~path_element_matches(){}

	bool operator()(const T& t)
	{
		if ( (!element_.id.empty()) && (element_.id == t->get_id()) ) {
			return true;
		}
		if (count_ == element_.position) {
			return true;
		}
		count_++;
		return false;
	}

private:
	int count_;
	path_element element_;
};


} //end of namespace ai


std::ostream &operator<<(std::ostream &o, const ai::path_element &e);

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
