/*
   Copyright (C) 2016 - 2017 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_TERRAIN_LAYERS_HPP_INCLUDED
#define GUI_DIALOGS_TERRAIN_LAYERS_HPP_INCLUDED

#include "gui/dialogs/modal_dialog.hpp"
#include "terrain/builder.hpp"

class CVideo;
class display;

namespace gui2
{
namespace dialogs
{

class terrain_layers : public modal_dialog
{
	// Since the 'display' type name clashes with the standard GUI2 static display
	// function name, we need a type alias.
	using display_t = ::display;

public:
	terrain_layers(display_t& disp, const map_location& loc);

	static void display(display_t& disp, const map_location& loc, CVideo& video)
	{
		terrain_layers(disp, loc).show(video);
	}

private:
	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from modal_dialog. */
	void pre_show(window& window);

	terrain_builder::tile* tile_;
	terrain_builder::tile::logs tile_logs_;
};

} // namespace dialogs
} // namespace gui2

#endif /* ! GUI_DIALOGS_TERRAIN_LAYERS_HPP_INCLUDED */
