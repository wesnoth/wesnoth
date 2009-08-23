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
	std::string property;
	std::string id;
	int position;
};


class component {
public:
	component() {};
	virtual ~component() {};
	virtual component* get_child(const path_element &child) = 0;
	virtual bool change_child(const path_element &child, const config &cfg) = 0;
	virtual bool add_child(const path_element &child, const config &cfg) = 0;
	virtual bool delete_child(const path_element &child) = 0;
};

class component_manager {
public:
	static bool add_component(component *root, const std::string &path, const config &cfg);
	static bool change_component(component *root, const std::string &path, const config &cfg);
	static bool delete_component(component *root, const std::string &path);
};

} //end of namespace ai

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
