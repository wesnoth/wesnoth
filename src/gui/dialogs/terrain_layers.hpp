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

#pragma once

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

	DEFINE_SIMPLE_DISPLAY_WRAPPER(terrain_layers)

private:
	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;

	/** Inherited from modal_dialog. */
	virtual void pre_show(window& window) override;

	terrain_builder::tile* tile_;
	terrain_builder::tile::logs tile_logs_;
};

} // namespace dialogs
} // namespace gui2
