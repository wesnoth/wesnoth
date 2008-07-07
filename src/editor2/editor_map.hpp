/* $Id$ */
/*
   Copyright (C) 2008 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef EDITOR2_EDITOR_MAP_HPP_INCLUDED
#define EDITOR2_EDITOR_MAP_HPP_INCLUDED

#include "../map.hpp"

namespace editor2 {

/**
 * This class adds extra editor-specific functionality to a normal gamemap
 */
	
class editor_map : public gamemap 
{
public:
	editor_map(const config& terrain_cfg, const std::string& data);
	std::vector<gamemap::location> get_tiles_in_radius(const gamemap::location& center, const unsigned int radius);
	static editor_map new_map(const config& terrain_cfg, size_t width, size_t height, t_translation::t_terrain filler);
	
	bool in_selection(const gamemap::location& loc) const;
	bool add_to_selection(const gamemap::location& loc);
	bool remove_from_selection(const gamemap::location& loc);
	const std::set<gamemap::location> selection() const { return selection_; }
	void clear_selection();
	void invert_selection();
	void select_all();
protected:
	std::set<gamemap::location> selection_;
};


} //end namespace editor2

#endif

