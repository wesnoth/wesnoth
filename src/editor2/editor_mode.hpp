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

#ifndef EDITOR2_EDITOR_MODE_HPP
#define EDITOR2_EDITOR_MODE_HPP

#include "editor_common.hpp"
#include "../map.hpp"
#include "../terrain.hpp"

namespace editor2 {
	
/**
 * This class encapsulates access to settings that define the editor's mode
 * of operation. This includes pallette terrain selection and the actual mouse
 * mode (e.g. paint/fill/select), the brush, but not settings specific to a
 * single mouse mode. It does not claim ownership of pointers passed to it.
 */
class editor_mode {
public:
	editor_mode() 
	: foreground_terrain_(t_translation::MOUNTAIN)
	, background_terrain_(t_translation::DEEP_WATER)
	, brush_(NULL)
	, mouse_action_(NULL)
	{
	}
	const t_translation::t_terrain& get_foreground_terrain() { return foreground_terrain_; }
	const t_translation::t_terrain& get_background_terrain() { return background_terrain_; }
	const brush* get_brush() { return brush_; }
	const mouse_action* get_mouse_action() { return mouse_action_; }
protected:
	void set_foreground_terrain(t_translation::t_terrain t) { foreground_terrain_ = t; }
	void set_background_terrain(t_translation::t_terrain t) { background_terrain_ = t; }
	void set_brush(brush* b) { brush_ = b; }
	void set_mouse_action(mouse_action* a) { mouse_action_ = a; }
private:
	t_translation::t_terrain foreground_terrain_;
	t_translation::t_terrain background_terrain_;
	brush* brush_;
	mouse_action* mouse_action_;
};

} //end namespace editor2
#endif
