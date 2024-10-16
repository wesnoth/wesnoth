/*
	Copyright (C) 2012 - 2024
	by Fabian Mueller <fabianmueller5@gmx.de>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

/**
 * Manage the item-palette in the editor.
 */

#pragma once

#include "editor/palette/editor_palettes.hpp"
#include "overlay.hpp"
class game_config_view;

namespace editor {

class editor_toolkit;

//std::string get_selected_terrain();

/** Palette where the terrain to be drawn can be selected. */
class item_palette : public editor_palette<overlay> {
public:

	item_palette(editor_display &gui, editor_toolkit &toolkit);

	virtual void setup(const game_config_view& cfg) override;

	virtual std::string get_help_string() const override;

private:

	virtual const std::string& get_id(const overlay& item) override;

	virtual void setup_item(
		const overlay& item,
		texture& item_base_image,
		texture& item_overlay_image,
		std::stringstream& tooltip
	) override;
};

}
