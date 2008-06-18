/* $Id$ */
/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//** @file editor_display.hpp */

#ifndef EDITOR_DISPLAY_H_INCLUDED
#define EDITOR_DISPLAY_H_INCLUDED

#include "../display.hpp"
class editor_display : public display
{
public:
	editor_display(CVideo& video, const gamemap& map, const config& theme_cfg,
			const config& cfg, const config& level);

	bool in_editor() const { return true; }

	/** Rebuild the dynamic terrain at the given location. */
	void rebuild_terrain(const gamemap::location &loc) {
        builder_.rebuild_terrain(loc);
    }
protected:
    void pre_draw();
    image::TYPE get_image_type(const gamemap::location& loc);
    const SDL_Rect& get_clip_rect();
    void draw_sidebar();
    

	/** Updates editor light levels from preferences */
	void update_light_levels();
private:
	int lr_, lg_, lb_;
};
#endif

