/*
   Copyright (C) 2009 - 2017 by Ignacio R. Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef STORYSCREEN_PARSER_HPP_INCLUDED
#define STORYSCREEN_PARSER_HPP_INCLUDED

#include <string>

class vconfig;

namespace storyscreen
{

/**
 * Small helper class to encapsulate the common logic for parsing storyscreen WML.
 */
class story_parser
{
public:
	story_parser() = default;

	story_parser(const story_parser&) = delete;
	story_parser& operator=(const story_parser&) = delete;

	/** Takes care of initializing and branching properties. */
	virtual void resolve_wml(const vconfig& cfg);

	/**
	 * May be implemented by derived classes to perform additional actions
	 * When executing @ref resolve_wml.
	 */
	virtual bool resolve_wml_helper(const std::string& key, const vconfig& node) = 0;
};

} // namespace storyscreen

#endif
